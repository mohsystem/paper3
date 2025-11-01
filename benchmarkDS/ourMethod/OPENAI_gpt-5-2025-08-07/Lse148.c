#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 280
#define MAX_MESSAGES 1000
#define MAX_OUTPUT_SIZE (1024 * 1024) /* 1 MiB cap for display */

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
} Message;

static Message* g_messages = NULL;
static size_t g_count = 0;
static size_t g_capacity = 0;

static int ensure_capacity(size_t needed) {
    if (needed <= g_capacity) return 1;
    size_t newcap = g_capacity == 0 ? 16 : g_capacity;
    while (newcap < needed) {
        if (newcap > MAX_MESSAGES / 2) { newcap = MAX_MESSAGES; break; }
        newcap *= 2;
    }
    if (newcap > MAX_MESSAGES) newcap = MAX_MESSAGES;
    Message* tmp = (Message*)realloc(g_messages, newcap * sizeof(Message));
    if (!tmp) return 0;
    g_messages = tmp;
    g_capacity = newcap;
    return 1;
}

static int validate_username(const char* username) {
    if (!username) return 0;
    size_t len = 0;
    for (const unsigned char* p = (const unsigned char*)username; *p; ++p) {
        unsigned char c = *p;
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
        len++;
        if (len > MAX_USERNAME_LEN) return 0;
    }
    return len >= 1 && len <= MAX_USERNAME_LEN;
}

static void sanitize_message(const char* in, char* out, size_t outsz) {
    if (!in || !out || outsz == 0) return;
    size_t w = 0;
    for (const unsigned char* p = (const unsigned char*)in; *p && w + 1 < outsz; ++p) {
        unsigned char c = *p;
        if (c == '\n' || c == '\t' || (c >= 0x20 && c != 0x7F)) {
            out[w++] = (char)c;
            if (w >= MAX_MESSAGE_LEN) break;
        }
    }
    /* trim trailing spaces except 
 or \t */
    while (w > 0 && out[w-1] != '\n' && out[w-1] != '\t' && isspace((unsigned char)out[w-1])) {
        w--;
    }
    out[w] = '\0';
}

static int validate_message(const char* message) {
    if (!message) return 0;
    size_t len = strlen(message);
    return len >= 1 && len <= MAX_MESSAGE_LEN;
}

int insert_user_message_in_db(const char* username, const char* message) {
    if (g_count >= MAX_MESSAGES) {
        /* drop oldest by shifting left by one */
        memmove(&g_messages[0], &g_messages[1], (g_count - 1) * sizeof(Message));
        g_count--;
    }
    if (!ensure_capacity(g_count + 1)) return 0;
    /* copy with bounds */
    snprintf(g_messages[g_count].username, sizeof(g_messages[g_count].username), "%s", username);
    snprintf(g_messages[g_count].message, sizeof(g_messages[g_count].message), "%s", message);
    g_count++;
    return 1;
}

const char* post(const char* username, const char* message) {
    static const char* REDIRECT = "REDIRECT: /";
    static const char* ERR_USER = "ERROR: invalid username";
    static const char* ERR_MSG  = "ERROR: invalid message";
    static const char* ERR_INT  = "ERROR: internal error";

    char ubuf[MAX_USERNAME_LEN + 1];
    char mbuf[MAX_MESSAGE_LEN + 1];

    if (!username) username = "";
    if (!message) message = "";

    /* trim username */
    const char* us = username;
    while (*us && isspace((unsigned char)*us)) us++;
    size_t ulen = strlen(us);
    while (ulen > 0 && isspace((unsigned char)us[ulen - 1])) ulen--;
    if (ulen > MAX_USERNAME_LEN) return ERR_USER;
    memcpy(ubuf, us, ulen);
    ubuf[ulen] = '\0';

    sanitize_message(message, mbuf, sizeof(mbuf));

    if (!validate_username(ubuf)) return ERR_USER;
    if (!validate_message(mbuf)) return ERR_MSG;
    if (!insert_user_message_in_db(ubuf, mbuf)) return ERR_INT;

    return REDIRECT;
}

static int safe_add_size_t(size_t a, size_t b, size_t* out) {
    if (SIZE_MAX - a < b) return 0;
    *out = a + b;
    return 1;
}

char* display_messages(void) {
    size_t needed = 1; /* for NUL */
    for (size_t i = 0; i < g_count; ++i) {
        size_t ulen = strcspn(g_messages[i].username, "\r");
        size_t mlen = strcspn(g_messages[i].message, "\r");
        size_t add = 0;
        /* ulen + 2 (": ") + mlen + 1 ("\n") */
        if (!safe_add_size_t(ulen, 2, &add)) return NULL;
        if (!safe_add_size_t(add, mlen, &add)) return NULL;
        if (!safe_add_size_t(add, 1, &add)) return NULL;
        if (!safe_add_size_t(needed, add, &needed)) return NULL;
        if (needed > MAX_OUTPUT_SIZE) return NULL;
    }

    char* buf = (char*)malloc(needed);
    if (!buf) return NULL;
    size_t w = 0;
    for (size_t i = 0; i < g_count; ++i) {
        const char* u = g_messages[i].username;
        const char* m = g_messages[i].message;
        /* write username without '\r' */
        for (size_t j = 0; u[j] && w + 1 < needed; ++j) {
            if (u[j] != '\r') buf[w++] = u[j];
        }
        if (w + 2 < needed) { buf[w++] = ':'; buf[w++] = ' '; }
        for (size_t j = 0; m[j] && w + 1 < needed; ++j) {
            if (m[j] != '\r') buf[w++] = m[j];
        }
        if (w + 1 < needed) buf[w++] = '\n';
    }
    if (w < needed) buf[w] = '\0';
    else buf[needed - 1] = '\0';
    return buf;
}

int main(void) {
    /* Test 1: valid */
    puts(post("alice", "Hello world!"));
    /* Test 2: long message */
    char longMsg[401];
    for (int i = 0; i < 400; ++i) longMsg[i] = 'x';
    longMsg[400] = '\0';
    puts(post("bob_123", longMsg));
    /* Test 3: invalid username */
    puts(post("bad user!", "This should fail"));
    /* Test 4: empty message */
    puts(post("charlie", "   "));
    /* Test 5: control + HTML */
    char ctrlMsg[] = "<script>alert(1)</script>\x01\x02ok";
    puts(post("dana", ctrlMsg));

    puts("=== Messages ===");
    char* out = display_messages();
    if (out) {
        fputs(out, stdout);
        free(out);
    } else {
        puts("ERROR: could not render messages");
    }
    /* cleanup */
    free(g_messages);
    g_messages = NULL;
    g_capacity = 0;
    g_count = 0;
    return 0;
}