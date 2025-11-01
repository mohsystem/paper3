#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USER_LEN 50
#define MAX_MSG_LEN 500

typedef struct {
    char* user;
    char* msg;
} Message;

static Message* DB = NULL;
static size_t DB_SIZE = 0;
static size_t DB_CAP = 0;

static void ensure_capacity(size_t need) {
    if (DB_CAP >= need) return;
    size_t newcap = DB_CAP ? DB_CAP * 2 : 8;
    if (newcap < need) newcap = need;
    Message* nd = (Message*)realloc(DB, newcap * sizeof(Message));
    if (!nd) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    DB = nd;
    DB_CAP = newcap;
}

static char* sanitize(const char* s, size_t maxLen) {
    if (!s) {
        char* e = (char*)calloc(1,1);
        return e;
    }
    char* tmp = (char*)malloc(maxLen + 1);
    if (!tmp) { fprintf(stderr, "OOM\n"); exit(1); }
    size_t n = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p && n < maxLen; ++p) {
        if (*p >= 32 && *p != 127) {
            tmp[n++] = (char)*p;
        }
    }
    tmp[n] = '\0';
    // trim
    size_t start = 0;
    while (start < n && isspace((unsigned char)tmp[start])) start++;
    size_t end = n;
    while (end > start && isspace((unsigned char)tmp[end-1])) end--;
    size_t len = end > start ? (end - start) : 0;
    char* out = (char*)malloc(len + 1);
    if (!out) { fprintf(stderr, "OOM\n"); exit(1); }
    if (len) memcpy(out, tmp + start, len);
    out[len] = '\0';
    free(tmp);
    return out;
}

static char* html_escape(const char* s) {
    if (!s) {
        char* e = (char*)calloc(1,1);
        return e;
    }
    size_t extra = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': extra += 4; break;  // &amp; (5-1)
            case '<':
            case '>': extra += 3; break;  // &lt; / &gt; (4-1)
            case '"': extra += 5; break;  // &quot; (6-1)
            case '\'': extra += 4; break; // &#39; (5-1)
            default: break;
        }
    }
    size_t len = strlen(s);
    char* out = (char*)malloc(len + extra + 1);
    if (!out) { fprintf(stderr, "OOM\n"); exit(1); }
    char* w = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(w, "&amp;", 5); w += 5; break;
            case '<': memcpy(w, "&lt;", 4); w += 4; break;
            case '>': memcpy(w, "&gt;", 4); w += 4; break;
            case '"': memcpy(w, "&quot;", 6); w += 6; break;
            case '\'': memcpy(w, "&#39;", 5); w += 5; break;
            default: *w++ = *p; break;
        }
    }
    *w = '\0';
    return out;
}

int insert_user_message_in_db(const char* username, const char* message) {
    char* user = sanitize(username, MAX_USER_LEN);
    char* msg = sanitize(message, MAX_MSG_LEN);
    if (user[0] == '\0' || msg[0] == '\0') {
        free(user);
        free(msg);
        return 0;
    }
    ensure_capacity(DB_SIZE + 1);
    DB[DB_SIZE].user = user;
    DB[DB_SIZE].msg = msg;
    DB_SIZE++;
    return 1;
}

// Simulated /post route
char* routePost(const char* username, const char* message) {
    insert_user_message_in_db(username, message);
    char* r = (char*)malloc(12);
    if (!r) { fprintf(stderr, "OOM\n"); exit(1); }
    strcpy(r, "REDIRECT: /");
    return r;
}

// Simulated / route - display messages
char* routeRoot(void) {
    const char* head = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body><h1>Messages</h1><ul>";
    const char* tail = "</ul></body></html>";
    size_t cap = strlen(head) + strlen(tail) + 1;
    for (size_t i = 0; i < DB_SIZE; ++i) {
        // estimate per message
        cap += 30 + strlen(DB[i].user) * 6 + strlen(DB[i].msg) * 6; // worst-case escaped
    }
    char* html = (char*)malloc(cap);
    if (!html) { fprintf(stderr, "OOM\n"); exit(1); }
    html[0] = '\0';
    strcat(html, head);
    for (size_t i = 0; i < DB_SIZE; ++i) {
        char* eu = html_escape(DB[i].user);
        char* em = html_escape(DB[i].msg);
        strcat(html, "<li><strong>");
        strcat(html, eu);
        strcat(html, "</strong>: ");
        strcat(html, em);
        strcat(html, "</li>");
        free(eu);
        free(em);
    }
    strcat(html, tail);
    return html;
}

int main(void) {
    // 5 test cases
    char* r1 = routePost("alice", "Hello, world!");
    printf("%s\n", r1); free(r1);

    char* r2 = routePost("bob", "Testing <b>bold</b> & \"quotes\"");
    printf("%s\n", r2); free(r2);

    char* r3 = routePost("charlie", "Line1\nLine2 should be sanitized for control chars.");
    printf("%s\n", r3); free(r3);

    // build a long message
    char longmsg[4096];
    longmsg[0] = '\0';
    for (int i = 0; i < 20; ++i) strcat(longmsg, "This is a very long message that will be trimmed if too long.");
    char* r4 = routePost("dave", longmsg);
    printf("%s\n", r4); free(r4);

    char* r5 = routePost(" eve ", "   spaced input   ");
    printf("%s\n", r5); free(r5);

    char* page = routeRoot();
    printf("%s\n", page);
    free(page);

    // cleanup DB
    for (size_t i = 0; i < DB_SIZE; ++i) {
        free(DB[i].user);
        free(DB[i].msg);
    }
    free(DB);
    return 0;
}