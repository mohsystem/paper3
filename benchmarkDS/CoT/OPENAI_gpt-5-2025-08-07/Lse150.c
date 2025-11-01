/* Chain-of-Through Steps (high-level, non-sensitive):
 * 1) Provide main_page() and post() to render/handle messages.
 * 2) Validate inputs, bound sizes, escape HTML output.
 * 3) Implement with strict checks and safe string handling.
 * 4) Review concatenations and allocations; avoid overflows.
 * 5) Final secure code with tests.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_MESSAGES 100
#define MAX_USERNAME 32
#define MAX_MESSAGE  500

typedef struct {
    char user[MAX_USERNAME + 1];
    char text[MAX_MESSAGE + 1];
} Message;

static Message MESSAGES[MAX_MESSAGES];
static size_t MESSAGE_COUNT = 0;

static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > MAX_USERNAME) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!( (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '_' || c == '-' )) {
            return 0;
        }
    }
    return 1;
}

static char* trim_copy(const char* s) {
    if (!s) return NULL;
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static char* sanitize_username(const char* username) {
    char* t = trim_copy(username);
    if (!t) return NULL;
    if (!is_valid_username(t)) { free(t); return NULL; }
    return t; // valid
}

static char* sanitize_message(const char* message) {
    if (!message) return NULL;
    size_t n = strlen(message);
    char* tmp = (char*)malloc(n + 1);
    if (!tmp) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char ch = (unsigned char)message[i];
        if (isprint(ch) || ch == '\n' || ch == '\t') tmp[j++] = (char)ch;
        else tmp[j++] = ' ';
    }
    tmp[j] = '\0';
    char* t = trim_copy(tmp);
    free(tmp);
    if (!t) return NULL;
    size_t len = strlen(t);
    if (len == 0) { free(t); return NULL; }
    if (len > MAX_MESSAGE) t[MAX_MESSAGE] = '\0';
    return t;
}

static char* html_escape(const char* s) {
    if (!s) return NULL;
    size_t need = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': need += 5; break;      // &amp;
            case '<': need += 4; break;      // &lt;
            case '>': need += 4; break;      // &gt;
            case '"': need += 6; break;      // &quot;
            case '\'': need += 6; break;     // &#x27;
            case '/': need += 5; break;      // &#x2F;
            default: need += 1; break;
        }
    }
    char* out = (char*)malloc(need + 1);
    if (!out) return NULL;
    char* q = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(q, "&amp;", 5); q += 5; break;
            case '<': memcpy(q, "&lt;", 4); q += 4; break;
            case '>': memcpy(q, "&gt;", 4); q += 4; break;
            case '"': memcpy(q, "&quot;", 6); q += 6; break;
            case '\'': memcpy(q, "&#x27;", 6); q += 6; break;
            case '/': memcpy(q, "&#x2F;", 5); q += 5; break;
            default: *q++ = *p; break;
        }
    }
    *q = '\0';
    return out;
}

int insert_user_message_in_db(const char* username, const char* message) {
    if (MESSAGE_COUNT >= MAX_MESSAGES) return 0;
    char* u = sanitize_username(username);
    char* m = sanitize_message(message);
    if (!u || !m) { if (u) free(u); if (m) free(m); return 0; }
    strncpy(MESSAGES[MESSAGE_COUNT].user, u, MAX_USERNAME);
    MESSAGES[MESSAGE_COUNT].user[MAX_USERNAME] = '\0';
    strncpy(MESSAGES[MESSAGE_COUNT].text, m, MAX_MESSAGE);
    MESSAGES[MESSAGE_COUNT].text[MAX_MESSAGE] = '\0';
    MESSAGE_COUNT++;
    free(u);
    free(m);
    return 1;
}

char* post(const char* username, const char* message) {
    int ok = insert_user_message_in_db(username, message);
    const char* resp = ok ? "REDIRECT: /" : "REDIRECT: /?error=invalid";
    char* out = (char*)malloc(strlen(resp) + 1);
    if (!out) return NULL;
    strcpy(out, resp);
    return out;
}

char* main_page() {
    const char* head = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>"
                       "<h1>Post a message</h1>"
                       "<form method=\"POST\" action=\"/post\">"
                       "<label>Username: <input type=\"text\" name=\"username\" maxlength=\"32\" required></label><br>"
                       "<label>Message: <textarea name=\"message\" maxlength=\"500\" required></textarea></label><br>"
                       "<button type=\"submit\">Send</button>"
                       "</form>"
                       "<h2>Recent Messages</h2><ul>";
    const char* tail = "</ul></body></html>";

    size_t total = strlen(head) + strlen(tail) + 1;
    // estimate messages size
    for (size_t i = 0; i < MESSAGE_COUNT; i++) {
        char* u = html_escape(MESSAGES[i].user);
        char* m = html_escape(MESSAGES[i].text);
        size_t add = strlen("<li><strong></strong>: </li>") + strlen(u) + strlen(m);
        total += add;
        free(u); free(m);
    }
    char* page = (char*)malloc(total);
    if (!page) return NULL;
    page[0] = '\0';
    strcat(page, head);
    for (size_t i = 0; i < MESSAGE_COUNT; i++) {
        char* u = html_escape(MESSAGES[i].user);
        char* m = html_escape(MESSAGES[i].text);
        size_t need = strlen(page) + strlen("<li><strong></strong>: </li>") + strlen(u) + strlen(m) + strlen(tail) + 1;
        char* grown = NULL;
        if (need > total) {
            total = need * 2;
            grown = (char*)realloc(page, total);
            if (!grown) { free(u); free(m); free(page); return NULL; }
            page = grown;
        }
        strcat(page, "<li><strong>");
        strcat(page, u);
        strcat(page, "</strong>: ");
        strcat(page, m);
        strcat(page, "</li>");
        free(u); free(m);
    }
    strcat(page, tail);
    return page;
}

int main(void) {
    // 5 Test cases
    printf("=== Test 1: Initial main page ===\n");
    char* p1 = main_page();
    if (p1) { printf("%.200s...\n", p1); free(p1); }

    printf("=== Test 2: Valid post ===\n");
    char* r2 = post("Alice", "Hello from C!");
    if (r2) { printf("%s\n", r2); free(r2); }

    printf("=== Test 3: XSS attempt ===\n");
    char* r3 = post("Bob", "<script>alert('x')</script>");
    if (r3) { printf("%s\n", r3); free(r3); }

    printf("=== Test 4: Invalid username ===\n");
    char* r4 = post("bad name", "should fail");
    if (r4) { printf("%s\n", r4); free(r4); }

    printf("=== Test 5: Main page after posts ===\n");
    char* p2 = main_page();
    if (p2) { printf("%.400s...\n", p2); free(p2); }

    return 0;
}