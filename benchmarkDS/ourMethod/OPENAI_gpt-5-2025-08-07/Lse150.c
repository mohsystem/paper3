#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 500
#define MAX_HTML_TOTAL 65536

typedef struct {
    char* username;
    char* message;
} Message;

typedef struct {
    int status_code;
    char* body;        // optional response body
    char location[64]; // for redirect
} Response;

typedef struct {
    char* username;
    char* message;
} Request;

static Message* DB = NULL;
static size_t DB_SIZE = 0;
static size_t DB_CAP = 0;

static void* xmalloc(size_t n) {
    void* p = malloc(n);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

static char* my_strdup(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)xmalloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t n = strlen(u);
    if (n == 0 || n > MAX_USERNAME_LEN) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '-' || c == '_')) return 0;
    }
    return 1;
}

static char* sanitize_message(const char* m) {
    if (!m) return NULL;
    // Remove control chars except 
 and \t, trim, enforce length
    char* buf = (char*)xmalloc(MAX_MESSAGE_LEN + 1);
    size_t count = 0;
    for (const unsigned char* p = (const unsigned char*)m; *p && count < MAX_MESSAGE_LEN; p++) {
        unsigned char c = *p;
        if (c == '\n' || c == '\t') {
            buf[count++] = (char)c;
        } else if (!(c < 32)) {
            buf[count++] = (char)c;
        }
    }
    buf[count] = '\0';

    // trim
    size_t start = 0;
    while (start < count && isspace((unsigned char)buf[start])) start++;
    size_t end = count;
    while (end > start && isspace((unsigned char)buf[end - 1])) end--;
    size_t len = (end > start) ? (end - start) : 0;
    if (len == 0) {
        free(buf);
        return NULL;
    }
    char* out = (char*)xmalloc(len + 1);
    memcpy(out, buf + start, len);
    out[len] = '\0';
    free(buf);
    return out;
}

// HTML escape
static char* html_escape(const char* s) {
    if (!s) return my_strdup("");
    size_t extra = 0;
    for (const char* p = s; *p; p++) {
        switch (*p) {
            case '&': extra += 4; break;  // & -> &amp; (extra 4 chars)
            case '<': extra += 3; break;  // < -> &lt; (extra 3)
            case '>': extra += 3; break;  // > -> &gt;
            case '"': extra += 5; break;  // " -> &quot; (extra 5)
            case '\'': extra += 4; break; // ' -> &#39; (extra 4)
            default: break;
        }
        if (extra > MAX_HTML_TOTAL) break;
    }
    size_t inlen = strlen(s);
    size_t outlen = inlen + extra;
    if (outlen > MAX_HTML_TOTAL) outlen = MAX_HTML_TOTAL; // clamp
    char* out = (char*)xmalloc(outlen + 1);
    size_t j = 0;
    for (size_t i = 0; i < inlen && j < outlen; i++) {
        char c = s[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#39;"; break;
            default: break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen <= outlen) {
                memcpy(out + j, rep, rlen);
                j += rlen;
            } else {
                break;
            }
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static int insert_user_message_in_db(const char* username, const char* message) {
    if (DB_SIZE == DB_CAP) {
        size_t newcap = DB_CAP == 0 ? 8 : DB_CAP * 2;
        if (newcap > 100000) return 0;
        Message* nd = (Message*)realloc(DB, newcap * sizeof(Message));
        if (!nd) return 0;
        DB = nd;
        DB_CAP = newcap;
    }
    DB[DB_SIZE].username = my_strdup(username);
    DB[DB_SIZE].message = my_strdup(message);
    DB_SIZE++;
    return 1;
}

// Simulated post handler
Response post(Request req) {
    // Trim username
    char* uname = req.username ? my_strdup(req.username) : my_strdup("");
    // trim leading
    size_t s = 0; while (uname[s] && isspace((unsigned char)uname[s])) s++;
    if (s > 0) memmove(uname, uname + s, strlen(uname + s) + 1);
    // trim trailing
    size_t n = strlen(uname);
    while (n > 0 && isspace((unsigned char)uname[n - 1])) { uname[n - 1] = '\0'; n--; }

    char* sanitized = sanitize_message(req.message);

    Response resp;
    resp.status_code = 0;
    resp.body = NULL;
    resp.location[0] = '\0';

    if (!is_valid_username(uname) || sanitized == NULL) {
        resp.status_code = 400;
        resp.body = my_strdup("Invalid input");
        free(uname);
        if (sanitized) free(sanitized);
        return resp;
    }

    if (!insert_user_message_in_db(uname, sanitized)) {
        resp.status_code = 500;
        resp.body = my_strdup("Internal error");
        free(uname);
        free(sanitized);
        return resp;
    }

    resp.status_code = 303;
    snprintf(resp.location, sizeof(resp.location), "%s", "/");
    free(uname);
    free(sanitized);
    return resp;
}

static char* get_main_page(void) {
    const char* head =
        "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"UTF-8\">"
        "<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<title>Secure Message Board</title></head><body>"
        "<h1>Message Board</h1>"
        "<form method=\"POST\" action=\"/post\" autocomplete=\"off\">"
        "<label>Username: <input type=\"text\" name=\"username\" required maxlength=\"";
    const char* mid1 = "\" pattern=\"[A-Za-z0-9_-]+\"></label><br>"
        "<label>Message:<br><textarea name=\"message\" required maxlength=\"";
    const char* mid2 = "\" rows=\"4\" cols=\"40\"></textarea></label><br>"
        "<button type=\"submit\">Send</button>"
        "</form><hr><h2>Recent Messages</h2><ul>";
    const char* tail = "</ul></body></html>";

    // Estimate size
    size_t total = strlen(head) + strlen(mid1) + strlen(mid2) + strlen(tail) + 64;
    for (size_t i = 0; i < DB_SIZE; i++) {
        // "<li><strong>": 12, ":</strong> ": 11, "</li>": 5
        total += 12 + 11 + 5;
        // escape strings
        char* eu = html_escape(DB[i].username);
        char* em = html_escape(DB[i].message);
        total += strlen(eu) + strlen(em);
        free(eu);
        free(em);
        if (total > MAX_HTML_TOTAL) break;
    }
    if (total > MAX_HTML_TOTAL) total = MAX_HTML_TOTAL;

    char* buf = (char*)xmalloc(total + 64);
    buf[0] = '\0';
    snprintf(buf, total + 64, "%s%d%s%d%s", head, MAX_USERNAME_LEN, mid1, MAX_MESSAGE_LEN, mid2);

    // append messages
    for (size_t i = 0; i < DB_SIZE; i++) {
        char* eu = html_escape(DB[i].username);
        char* em = html_escape(DB[i].message);
        strncat(buf, "<li><strong>", total);
        strncat(buf, eu, total);
        strncat(buf, ":</strong> ", total);
        strncat(buf, em, total);
        strncat(buf, "</li>", total);
        free(eu);
        free(em);
    }
    strncat(buf, tail, total);
    return buf;
}

static void free_db(void) {
    for (size_t i = 0; i < DB_SIZE; i++) {
        free(DB[i].username);
        free(DB[i].message);
    }
    free(DB);
    DB = NULL;
    DB_SIZE = DB_CAP = 0;
}

int main(void) {
    // 1) Valid post
    Request r1 = { "alice", "Hello, world!" };
    Response a = post(r1);
    printf("HTTP/1.1 %d\n", a.status_code);
    if (a.status_code == 303) printf("Location: %s\n", a.location);
    printf("\n%s\n", a.body ? a.body : "");
    free(a.body);

    // 2) Valid post with HTML needing escape
    Request r2 = { "Bob_123", "Hi <script>alert('x')</script> & welcome!" };
    Response b = post(r2);
    printf("HTTP/1.1 %d\n", b.status_code);
    if (b.status_code == 303) printf("Location: %s\n", b.location);
    printf("\n%s\n", b.body ? b.body : "");
    free(b.body);

    // 3) Invalid username
    Request r3 = { "bad!user", "This should fail." };
    Response c = post(r3);
    printf("HTTP/1.1 %d\n", c.status_code);
    if (c.status_code == 303) printf("Location: %s\n", c.location);
    printf("\n%s\n", c.body ? c.body : "");
    free(c.body);

    // 4) Too long message
    char* longmsg = (char*)xmalloc(601);
    memset(longmsg, 'x', 600);
    longmsg[600] = '\0';
    Request r4 = { "charlie", longmsg };
    Response d = post(r4);
    printf("HTTP/1.1 %d\n", d.status_code);
    if (d.status_code == 303) printf("Location: %s\n", d.location);
    printf("\n%s\n", d.body ? d.body : "");
    free(d.body);
    free(longmsg);

    // 5) Empty after trim
    Request r5 = { "david", "   \n\t   " };
    Response e = post(r5);
    printf("HTTP/1.1 %d\n", e.status_code);
    if (e.status_code == 303) printf("Location: %s\n", e.location);
    printf("\n%s\n", e.body ? e.body : "");
    free(e.body);

    // Render main page
    char* page = get_main_page();
    printf("---- MAIN PAGE HTML ----\n%s\n---- END PAGE ----\n", page);
    free(page);

    free_db();
    return 0;
}