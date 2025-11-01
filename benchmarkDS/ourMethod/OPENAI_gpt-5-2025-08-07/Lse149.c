#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Message entity */
typedef struct {
    char *username;
    char *message;
} Message;

/* In-memory DB with bounded size */
typedef struct {
    Message *messages;
    size_t count;
    size_t capacity;
} InMemoryDB;

/* Initialize DB */
static int db_init(InMemoryDB *db, size_t capacity) {
    if (!db) return 0;
    if (capacity == 0) capacity = 1;
    db->messages = (Message*)calloc(capacity, sizeof(Message));
    if (!db->messages) return 0;
    db->count = 0;
    db->capacity = capacity;
    return 1;
}

/* Free DB */
static void db_free(InMemoryDB *db) {
    if (!db) return;
    for (size_t i = 0; i < db->count; i++) {
        free(db->messages[i].username);
        free(db->messages[i].message);
    }
    free(db->messages);
    db->messages = NULL;
    db->count = 0;
    db->capacity = 0;
}

/* Validate username */
static int valid_username(const char *u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > 20) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(c == '_' || isalnum((unsigned char)c))) return 0;
    }
    return 1;
}

/* Validate message */
static int valid_message(const char *m) {
    if (!m) return 0;
    size_t len = strlen(m);
    if (len < 1 || len > 280) return 0;
    return 1;
}

/* Safe strdup */
static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *p = (char*)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, s, len);
    p[len] = '\0';
    return p;
}

/* Trim spaces */
static char *trim_inplace(char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) start++;
    size_t end = len;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\n' || s[end - 1] == '\r')) end--;
    if (start > 0) memmove(s, s + start, end - start);
    s[end - start] = '\0';
    return s;
}

/* Insert message into DB, evict oldest if full */
int insert_user_message_in_db(InMemoryDB *db, const char *username, const char *message) {
    if (!db || !username || !message) return 0;

    char *u = safe_strdup(username);
    char *m = safe_strdup(message);
    if (!u || !m) {
        free(u);
        free(m);
        return 0;
    }
    trim_inplace(u);
    trim_inplace(m);

    if (!valid_username(u) || !valid_message(m)) {
        free(u);
        free(m);
        return 0;
    }

    if (db->count >= db->capacity) {
        /* Evict oldest */
        free(db->messages[0].username);
        free(db->messages[0].message);
        for (size_t i = 1; i < db->count; i++) {
            db->messages[i - 1] = db->messages[i];
        }
        db->count--;
    }

    db->messages[db->count].username = u;
    db->messages[db->count].message = m;
    db->count++;
    return 1;
}

/* HTML escape */
static char *escape_html(const char *s) {
    if (!s) {
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(s);
    /* Worst case every char becomes 6 chars (&quot;) -> allocate 6x + 1 with cap */
    size_t cap = len * 6 + 1;
    if (cap > 1024 * 1024) cap = 1024 * 1024; /* safety cap */
    char *out = (char*)malloc(cap);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        const char *rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#39;"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen + 1 >= cap) break; /* prevent overflow */
            memcpy(out + j, rep, rlen);
            j += rlen;
        } else {
            if ((unsigned char)c < 0x20 && c != '\n' && c != '\t') {
                continue; /* skip control chars */
            }
            if (j + 2 >= cap) break;
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

/* Response structure */
typedef struct {
    int statusCode;
    char *headers;
    char *body;
} Response;

/* Create text response */
static Response text_resp(int code, const char *body) {
    Response r;
    r.statusCode = code;
    r.headers = safe_strdup("Content-Type: text/plain; charset=utf-8");
    r.body = safe_strdup(body ? body : "");
    return r;
}

/* Create html response */
static Response html_resp(int code, const char *body) {
    Response r;
    r.statusCode = code;
    r.headers = safe_strdup("Content-Type: text/html; charset=utf-8");
    r.body = safe_strdup(body ? body : "");
    return r;
}

/* Create redirect response */
static Response redirect_resp(const char *location) {
    Response r;
    r.statusCode = 303;
    size_t hl = strlen("Location: ") + strlen(location) + 1;
    r.headers = (char*)malloc(hl);
    if (r.headers) {
        snprintf(r.headers, hl, "Location: %s", location);
    }
    r.body = safe_strdup("");
    return r;
}

/* Free response */
static void resp_free(Response *r) {
    if (!r) return;
    free(r->headers);
    free(r->body);
    r->headers = NULL;
    r->body = NULL;
}

/* Render root page */
static Response handle_root(const InMemoryDB *db) {
    /* Build HTML listing messages */
    const char *head = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>"
                       "<h1>Message Board</h1>"
                       "<form method=\"POST\" action=\"/post\" onsubmit=\"return /^[A-Za-z0-9_]{1,20}$/.test(this.username.value);\">"
                       "Username: <input name=\"username\" maxlength=\"20\" required pattern=\"[A-Za-z0-9_]+\"> "
                       "Message: <input name=\"message\" maxlength=\"280\" required> "
                       "<button type=\"submit\">Post</button></form>"
                       "<h2>Messages</h2><ul>";
    const char *tail = "</ul></body></html>";

    size_t bufSz = strlen(head) + strlen(tail) + 1;
    for (size_t i = 0; i < db->count; i++) {
        /* Estimate per item conservatively */
        bufSz += 32;
        if (db->messages[i].username) bufSz += strlen(db->messages[i].username) * 6;
        if (db->messages[i].message) bufSz += strlen(db->messages[i].message) * 6;
    }
    if (bufSz > 2 * 1024 * 1024) bufSz = 2 * 1024 * 1024; /* safety cap */

    char *html = (char*)malloc(bufSz);
    if (!html) return text_resp(500, "Internal Server Error");

    size_t o = 0;
    o += snprintf(html + o, bufSz - o, "%s", head);
    for (size_t i = 0; i < db->count; i++) {
        char *eu = escape_html(db->messages[i].username ? db->messages[i].username : "");
        char *em = escape_html(db->messages[i].message ? db->messages[i].message : "");
        if (!eu || !em) {
            free(eu);
            free(em);
            free(html);
            return text_resp(500, "Internal Server Error");
        }
        o += snprintf(html + o, (o < bufSz ? bufSz - o : 0), "<li><strong>%s</strong>: %s</li>", eu, em);
        free(eu);
        free(em);
        if (o >= bufSz) break;
    }
    if (o < bufSz) snprintf(html + o, bufSz - o, "%s", tail);
    Response r = html_resp(200, html);
    free(html);
    return r;
}

/* Handle /post */
static Response handle_post(InMemoryDB *db, const char *username, const char *message) {
    if (!insert_user_message_in_db(db, username, message)) {
        return text_resp(400, "Invalid input");
    }
    return redirect_resp("/");
}

/* Simulated router */
static Response route(InMemoryDB *db, const char *method, const char *path, const char *username, const char *message) {
    if (!method || !path) return text_resp(400, "Bad Request");
    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        return handle_root(db);
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/post") == 0) {
        return handle_post(db, username, message);
    }
    return text_resp(404, "Not Found");
}

/* Print response */
static void print_response(const Response *r) {
    if (!r) return;
    printf("Status: %d\n", r->statusCode);
    printf("%s\n\n", r->headers ? r->headers : "");
    printf("%s\n", r->body ? r->body : "");
}

int main(void) {
    InMemoryDB db;
    if (!db_init(&db, 1000)) {
        fprintf(stderr, "Failed to init DB\n");
        return 1;
    }

    /* Test 1: Valid post then GET */
    {
        Response r1 = route(&db, "POST", "/post", "alice", "Hello, world!");
        printf("Test 1 POST /post\n");
        print_response(&r1);
        resp_free(&r1);

        Response r2 = route(&db, "GET", "/", NULL, NULL);
        printf("GET / after Test 1\n");
        print_response(&r2);
        resp_free(&r2);
        printf("------------------------------------------------------------\n");
    }

    /* Test 2: Invalid username */
    {
        Response r = route(&db, "POST", "/post", "bob!", "Should fail");
        printf("Test 2 POST /post invalid username\n");
        print_response(&r);
        resp_free(&r);
        printf("------------------------------------------------------------\n");
    }

    /* Test 3: Too long message */
    {
        char longMsg[282];
        memset(longMsg, 'x', 281);
        longMsg[281] = '\0';
        Response r = route(&db, "POST", "/post", "charlie", longMsg);
        printf("Test 3 POST /post long message\n");
        print_response(&r);
        resp_free(&r);
        printf("------------------------------------------------------------\n");
    }

    /* Test 4: XSS attempt */
    {
        Response r1 = route(&db, "POST", "/post", "dana", "<script>alert(1)</script> ok");
        printf("Test 4 POST /post XSS attempt\n");
        print_response(&r1);
        resp_free(&r1);

        Response r2 = route(&db, "GET", "/", NULL, NULL);
        printf("GET / after Test 4\n");
        print_response(&r2);
        resp_free(&r2);
        printf("------------------------------------------------------------\n");
    }

    /* Test 5: Another valid post and list */
    {
        Response r1 = route(&db, "POST", "/post", "eve_123", "Good day!");
        printf("Test 5 POST /post valid again\n");
        print_response(&r1);
        resp_free(&r1);

        Response r2 = route(&db, "GET", "/", NULL, NULL);
        printf("GET / after Test 5\n");
        print_response(&r2);
        resp_free(&r2);
        printf("------------------------------------------------------------\n");
    }

    db_free(&db);
    return 0;
}