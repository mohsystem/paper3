#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

/*
 Chain-of-Through (comments):
 1) Implement post(): get username/message from request, insert, redirect, display.
 2) Security: validate, length bounds, escape HTML, strip controls, safe buffers.
 3) Secure coding: bounded copies, size checks, malloc with size calc, no unsafe funcs.
 4) Review: generic error redirect, avoid sensitive logs.
 5) Final: safe functions.
*/

#define MAX_MSGS 512
#define MAX_USER_LEN 32
#define MAX_RAW_MSG_LEN 1000
#define MAX_STORED_USER 64
#define MAX_STORED_MSG 2048

typedef struct {
    char username[MAX_STORED_USER];
    char message[MAX_STORED_MSG];
    time_t ts;
} Message;

typedef struct {
    char username[128];
    char message[2048];
} Request;

static Message DB[MAX_MSGS];
static size_t DB_COUNT = 0;

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t n = strlen(u);
    if (n == 0 || n > MAX_USER_LEN) return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c=='_' || c=='-' || c=='.' || c==' ')) return 0;
    }
    return 1;
}

static int is_valid_message(const char* m) {
    if (m == NULL) return 0;
    size_t n = strlen(m);
    if (n == 0 || n > MAX_RAW_MSG_LEN) return 0;
    return 1;
}

static void trim(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end-1])) end--;
    if (start > 0) memmove(s, s+start, end - start);
    s[end - start] = '\0';
}

static void escape_html(const char* in, char* out, size_t outcap) {
    // Escape &, <, >, ", ' and drop dangerous control chars except 
 \r \t
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)in[i];
        const char* rep = NULL;
        char buf[8];
        if (c == '&') rep = "&amp;";
        else if (c == '<') rep = "&lt;";
        else if (c == '>') rep = "&gt;";
        else if (c == '"') rep = "&quot;";
        else if (c == '\'') rep = "&#39;";
        else if ((c < 32) && !(c=='\n' || c=='\r' || c=='\t')) {
            continue; // skip other control chars
        }

        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen >= outcap) break;
            memcpy(out + j, rep, rlen);
            j += rlen;
        } else {
            if (j + 1 >= outcap) break;
            out[j++] = (char)c;
        }
    }
    if (j < outcap) out[j] = '\0';
    else out[outcap-1] = '\0';
}

int insert_user_message_in_db(const char* usernameRaw, const char* messageRaw) {
    if (!usernameRaw || !messageRaw) return 0;
    char u[128]; char m[2048];
    strncpy(u, usernameRaw, sizeof(u)-1); u[sizeof(u)-1] = '\0';
    strncpy(m, messageRaw, sizeof(m)-1); m[sizeof(m)-1] = '\0';
    trim(u); trim(m);

    if (!is_valid_username(u)) return 0;
    if (!is_valid_message(m)) return 0;
    if (DB_COUNT >= MAX_MSGS) return 0;

    Message msg;
    escape_html(u, msg.username, sizeof(msg.username));
    escape_html(m, msg.message, sizeof(msg.message));
    msg.ts = time(NULL);

    DB[DB_COUNT++] = msg;
    return 1;
}

const char* post(const Request* req) {
    if (!req) return "REDIRECT: /?error=invalid";
    int ok = insert_user_message_in_db(req->username, req->message);
    return ok ? "REDIRECT: /" : "REDIRECT: /?error=invalid";
}

char* display_messages(void) {
    // Compute required size
    const char* header = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body><h1>Messages</h1><ul>";
    const char* footer = "</ul></body></html>";
    size_t total = strlen(header) + strlen(footer) + 1;
    for (size_t i = 0; i < DB_COUNT; ++i) {
        // Each item approx: "<li><strong></strong> []: </li>"
        total += strlen(DB[i].username) + strlen(DB[i].message) + 64;
    }
    char* out = (char*)malloc(total);
    if (!out) return NULL;
    out[0] = '\0';
    strncat(out, header, total - strlen(out) - 1);
    for (size_t i = 0; i < DB_COUNT; ++i) {
        char timebuf[32];
        struct tm tminfo;
        #if defined(_WIN32)
            localtime_s(&tminfo, &DB[i].ts);
        #else
            localtime_r(&DB[i].ts, &tminfo);
        #endif
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tminfo);
        char item[4096];
        snprintf(item, sizeof(item), "<li><strong>%s</strong> [%s]: %s</li>",
                 DB[i].username, timebuf, DB[i].message);
        strncat(out, item, total - strlen(out) - 1);
    }
    strncat(out, footer, total - strlen(out) - 1);
    return out;
}

int main(void) {
    // 5 test cases
    Request r1 = { "Alice", "Hello world!" };
    printf("%s\n", post(&r1));

    Request r2 = { "Bob_123", "<script>alert('x')</script> & text" };
    printf("%s\n", post(&r2));

    Request r3 = { "Bad#Name", "Should fail" };
    printf("%s\n", post(&r3));

    Request r4 = { "Carol", "Line1\nLine2\tTabbed" };
    printf("%s\n", post(&r4));

    char bigmsg[1200];
    memset(bigmsg, 'A', 1100);
    bigmsg[1100] = '\0';
    Request r5; strncpy(r5.username, "Dave", sizeof(r5.username)-1); r5.username[sizeof(r5.username)-1]='\0';
    strncpy(r5.message, bigmsg, sizeof(r5.message)-1); r5.message[sizeof(r5.message)-1]='\0';
    printf("%s\n", post(&r5));

    char* page = display_messages();
    if (page) {
        printf("%s\n", page);
        free(page);
    }
    return 0;
}