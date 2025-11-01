#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_MESSAGE_LEN 500
#define MAX_USERNAME_LEN 32
#define MAX_MESSAGES 1000

typedef struct {
    char* username;
    char* message;
    time_t ts;
} Message;

static Message DB[MAX_MESSAGES];
static size_t DB_COUNT = 0;

static int is_valid_username(const char* username) {
    if (!username) return 0;
    size_t len = 0;
    while (username[len] != '\0') {
        unsigned char c = (unsigned char)username[len];
        if (!(isalnum(c) || c == '_' || c == '.' || c == '-')) {
            return 0;
        }
        len++;
        if (len > MAX_USERNAME_LEN) return 0;
    }
    return len >= 1 && len <= MAX_USERNAME_LEN;
}

static char* str_trim_copy(const char* s) {
    if (!s) return NULL;
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static char* sanitize_username(const char* username) {
    char* trimmed = str_trim_copy(username);
    if (!trimmed) return NULL;
    if (!is_valid_username(trimmed)) {
        free(trimmed);
        return NULL;
    }
    return trimmed;
}

static char* sanitize_message(const char* message) {
    if (!message) return NULL;
    // Normalize CRLF/CR to LF and build filtered string
    size_t in_len = strlen(message);
    char* temp = (char*)malloc(in_len + 1);
    if (!temp) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < in_len; i++) {
        char c = message[i];
        if (c == '\r') {
            if (i + 1 < in_len && message[i+1] == '\n') {
                // skip adding extra, we'll add one 

            }
            temp[j++] = '\n';
        } else {
            temp[j++] = c;
        }
    }
    temp[j] = '\0';

    // Trim
    char* trimmed = str_trim_copy(temp);
    free(temp);
    if (!trimmed) return NULL;
    if (trimmed[0] == '\0') {
        free(trimmed);
        return NULL;
    }

    // Limit length
    size_t len = strlen(trimmed);
    if (len > MAX_MESSAGE_LEN) {
        trimmed[MAX_MESSAGE_LEN] = '\0';
        len = MAX_MESSAGE_LEN;
    }

    // Remove control chars except 
 and \t
    char* clean = (char*)malloc(len + 1);
    if (!clean) {
        free(trimmed);
        return NULL;
    }
    size_t k = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)trimmed[i];
        if (c == '\n' || c == '\t' || (c >= 0x20 && c != 0x7F)) {
            clean[k++] = (char)c;
        }
    }
    clean[k] = '\0';
    free(trimmed);
    if (k == 0) {
        free(clean);
        return NULL;
    }
    return clean;
}

static int insert_user_message_in_db(const char* username, const char* message) {
    if (DB_COUNT >= MAX_MESSAGES) return 0;
    DB[DB_COUNT].username = strdup(username);
    DB[DB_COUNT].message = strdup(message);
    if (!DB[DB_COUNT].username || !DB[DB_COUNT].message) {
        free(DB[DB_COUNT].username);
        free(DB[DB_COUNT].message);
        return 0;
    }
    DB[DB_COUNT].ts = time(NULL);
    DB_COUNT++;
    return 1;
}

static char* html_escape(const char* s) {
    if (!s) return strdup("");
    size_t cap = strlen(s) * 6 + 1; // worst case
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            case '/': rep = "&#x2F;"; break;
            default: break;
        }
        if (rep) {
            size_t rl = strlen(rep);
            if (j + rl + 1 > cap) {
                cap = (cap + rl + 64) * 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            memcpy(out + j, rep, rl);
            j += rl;
        } else {
            if (j + 2 > cap) {
                cap = (cap + 64) * 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

char* display_messages(void) {
    const char* header = "<html><body>\n<h2>Messages</h2>\n<ul>\n";
    const char* footer = "</ul>\n</body></html>";
    size_t cap = strlen(header) + strlen(footer) + 64;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;
    size_t len = 0;
    memcpy(buf + len, header, strlen(header));
    len += strlen(header);
    for (size_t i = 0; i < DB_COUNT; i++) {
        char* u = html_escape(DB[i].username);
        char* m = html_escape(DB[i].message);
        if (!u || !m) {
            free(u); free(m); free(buf);
            return NULL;
        }
        const char* li_start = "<li><strong>";
        const char* mid = ":</strong> ";
        const char* li_end = "</li>\n";
        size_t need = strlen(li_start) + strlen(u) + strlen(mid) + strlen(m) + strlen(li_end) + 1;
        if (len + need + strlen(footer) + 1 > cap) {
            cap = (cap + need + strlen(footer) + 128) * 2;
            char* tmp = (char*)realloc(buf, cap);
            if (!tmp) { free(u); free(m); free(buf); return NULL; }
            buf = tmp;
        }
        memcpy(buf + len, li_start, strlen(li_start)); len += strlen(li_start);
        memcpy(buf + len, u, strlen(u)); len += strlen(u);
        memcpy(buf + len, mid, strlen(mid)); len += strlen(mid);
        memcpy(buf + len, m, strlen(m)); len += strlen(m);
        memcpy(buf + len, li_end, strlen(li_end)); len += strlen(li_end);
        free(u);
        free(m);
    }
    if (len + strlen(footer) + 1 > cap) {
        cap = len + strlen(footer) + 1;
        char* tmp = (char*)realloc(buf, cap);
        if (!tmp) { free(buf); return NULL; }
        buf = tmp;
    }
    memcpy(buf + len, footer, strlen(footer));
    len += strlen(footer);
    buf[len] = '\0';
    return buf;
}

char* render_main_page(void) {
    return display_messages();
}

char* post(const char* username, const char* message) {
    char* u = sanitize_username(username);
    char* m = sanitize_message(message);
    if (u && m) {
        insert_user_message_in_db(u, m);
    }
    free(u);
    free(m);
    // Return a newly allocated string as the "redirect"
    char* redirect = (char*)malloc(11);
    if (!redirect) return NULL;
    memcpy(redirect, "redirect:/", 10);
    redirect[10] = '\0';
    return redirect;
}

int main(void) {
    // 5 test cases
    char* r1 = post("alice", "Hello, world!");
    char* r2 = post("bob_42", "This is a test message.");
    char* r3 = post("charlie", "<script>alert('xss');</script> Stay safe!");
    char* r4 = post("invalid name", "Should be rejected due to invalid username.");
    char* r5 = post("david", "");
    if (r1) { printf("%s\n", r1); free(r1); }
    if (r2) { printf("%s\n", r2); free(r2); }
    if (r3) { printf("%s\n", r3); free(r3); }
    if (r4) { printf("%s\n", r4); free(r4); }
    if (r5) { printf("%s\n", r5); free(r5); }

    char* page = render_main_page();
    if (page) {
        puts(page);
        free(page);
    }

    // Cleanup allocated messages
    for (size_t i = 0; i < DB_COUNT; i++) {
        free(DB[i].username);
        free(DB[i].message);
    }
    return 0;
}