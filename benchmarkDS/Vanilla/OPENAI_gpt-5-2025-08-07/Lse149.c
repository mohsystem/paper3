#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_MESSAGES 1000
#define MAX_STR 256

typedef struct {
    char username[MAX_STR];
    char message[MAX_STR];
    long ts;
} Message;

static Message DB[MAX_MESSAGES];
static int DB_COUNT = 0;

int insert_user_message_in_db(const char* username, const char* message) {
    if (!username || !message) return 0;
    if (strlen(username) == 0 || strlen(message) == 0) return 0;
    if (DB_COUNT >= MAX_MESSAGES) return 0;
    strncpy(DB[DB_COUNT].username, username, MAX_STR - 1);
    DB[DB_COUNT].username[MAX_STR - 1] = '\0';
    strncpy(DB[DB_COUNT].message, message, MAX_STR - 1);
    DB[DB_COUNT].message[MAX_STR - 1] = '\0';
    DB[DB_COUNT].ts = time(NULL);
    DB_COUNT++;
    return 1;
}

void escapeHtml(const char* src, char* dst, size_t dstsz) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j + 1 < dstsz; ++i) {
        char c = src[i];
        const char* repl = NULL;
        switch (c) {
            case '&': repl = "&amp;"; break;
            case '<': repl = "&lt;";  break;
            case '>': repl = "&gt;";  break;
            case '"': repl = "&quot;"; break;
            case '\'': repl = "&#x27;"; break;
            default: repl = NULL;
        }
        if (repl) {
            size_t rlen = strlen(repl);
            if (j + rlen >= dstsz) break;
            memcpy(dst + j, repl, rlen);
            j += rlen;
        } else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
}

void route_root(char* out, size_t outsz) {
    // Build a simple HTML page
    const char* header = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Messages</title></head><body><h1>Messages</h1><ul>";
    const char* footer = "</ul><h2>Post a message</h2><form method='POST' action='/post'>Username: <input name='username' required/> <br/>Message: <input name='message' required/> <br/><button type='submit'>Post</button></form></body></html>";
    strncpy(out, header, outsz - 1);
    out[outsz - 1] = '\0';
    size_t len = strlen(out);

    for (int i = 0; i < DB_COUNT; ++i) {
        char uesc[4 * MAX_STR], mesc[4 * MAX_STR];
        escapeHtml(DB[i].username, uesc, sizeof(uesc));
        escapeHtml(DB[i].message, mesc, sizeof(mesc));
        char line[4 * MAX_STR + 64];
        snprintf(line, sizeof(line), "<li><strong>%s:</strong> %s</li>", uesc, mesc);
        size_t l = strlen(line);
        if (len + l + 1 < outsz) {
            memcpy(out + len, line, l);
            len += l;
            out[len] = '\0';
        } else {
            break;
        }
    }
    size_t flen = strlen(footer);
    if (len + flen + 1 < outsz) {
        memcpy(out + len, footer, flen);
        len += flen;
        out[len] = '\0';
    }
}

const char* route_post(const char* username, const char* message) {
    insert_user_message_in_db(username, message);
    return "REDIRECT /";
}

int main() {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        char u[32], m[64];
        snprintf(u, sizeof(u), "user%d", i);
        snprintf(m, sizeof(m), "hello world %d", i);
        const char* res = route_post(u, m);
        printf("POST %d response: %s\n", i, res);
    }
    char page[65536];
    route_root(page, sizeof(page));
    printf("%s\n", page);
    return 0;
}