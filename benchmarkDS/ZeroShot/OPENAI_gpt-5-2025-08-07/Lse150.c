#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

#define MAX_DB_RECORDS 256
#define MAX_RECORD_LEN 560

typedef struct {
    char records[MAX_DB_RECORDS][MAX_RECORD_LEN];
    size_t count;
} DB;

char* html_escape(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(s);
    // Worst case every char becomes 6 chars
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        const char* rep = NULL;
        if (c == '&') rep = "&amp;";
        else if (c == '<') rep = "&lt;";
        else if (c == '>') rep = "&gt;";
        else if (c == '"') rep = "&quot;";
        else if (c == '\'') rep = "&#x27;";
        else if (c == '/') rep = "&#x2F;";
        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen >= cap) {
                cap *= 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            memcpy(out + j, rep, rlen);
            j += rlen;
        } else {
            if (j + 1 >= cap) {
                cap *= 2;
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

static void trim_inplace(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) start++;
    size_t end = len;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n')) end--;
    if (start > 0) memmove(s, s + start, end - start);
    s[end - start] = '\0';
}

int validate_username(const char* username) {
    if (!username) return 0;
    char* tmp = strdup(username);
    if (!tmp) return 0;
    trim_inplace(tmp);
    size_t len = strlen(tmp);
    if (len < 3 || len > 32) { free(tmp); return 0; }
    for (size_t i = 0; i < len; i++) {
        char c = tmp[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-')) { free(tmp); return 0; }
    }
    free(tmp);
    return 1;
}

// Returns newly allocated sanitized message, or NULL on failure
char* sanitize_message(const char* message) {
    if (!message) return NULL;
    char* tmp = strdup(message);
    if (!tmp) return NULL;
    trim_inplace(tmp);
    size_t cap = 512;
    char* out = (char*)malloc(cap);
    if (!out) { free(tmp); return NULL; }
    size_t j = 0;
    for (size_t i = 0; tmp[i] != '\0'; i++) {
        unsigned char c = (unsigned char)tmp[i];
        if (c == '\n' || (c >= 0x20 && c != 0x7F)) {
            if (j + 1 >= cap) {
                cap *= 2;
                char* t2 = (char*)realloc(out, cap);
                if (!t2) { free(out); free(tmp); return NULL; }
                out = t2;
            }
            out[j++] = (char)c;
            if (j > 500) { free(out); free(tmp); return NULL; }
        }
    }
    free(tmp);
    if (j == 0) { free(out); return NULL; }
    out[j] = '\0';
    return out;
}

int constant_time_equals(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char diff = 0;
    for (size_t i = 0; i < la; i++) {
        diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    return diff == 0;
}

int insert_user_message_in_db(DB* db, const char* username, const char* message) {
    if (!db || !username || !message) return 0;
    if (db->count >= MAX_DB_RECORDS) return 0;
    char rec[MAX_RECORD_LEN];
    rec[0] = '\0';
    strncat(rec, username, MAX_RECORD_LEN - 1);
    strncat(rec, ": ", MAX_RECORD_LEN - strlen(rec) - 1);
    strncat(rec, message, MAX_RECORD_LEN - strlen(rec) - 1);
    strncpy(db->records[db->count], rec, MAX_RECORD_LEN - 1);
    db->records[db->count][MAX_RECORD_LEN - 1] = '\0';
    db->count++;
    return 1;
}

char* render_main_page(const char* csrfToken) {
    char* safeToken = html_escape(csrfToken);
    if (!safeToken) return NULL;
    const char* pre =
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Main</title></head><body>"
        "<h1>Submit a message</h1>"
        "<form method=\"post\" action=\"/post\" autocomplete=\"off\">"
        "<label for=\"username\">Username</label>"
        "<input type=\"text\" id=\"username\" name=\"username\" required minlength=\"3\" maxlength=\"32\" pattern=\"[A-Za-z0-9_\\-]+\">"
        "<br>"
        "<label for=\"message\">Message</label>"
        "<textarea id=\"message\" name=\"message\" required maxlength=\"500\"></textarea>"
        "<input type=\"hidden\" name=\"csrf\" value=\"";
    const char* post =
        "\">"
        "<br><button type=\"submit\">Send</button>"
        "</form>"
        "</body></html>";
    size_t cap = strlen(pre) + strlen(safeToken) + strlen(post) + 1;
    char* html = (char*)malloc(cap);
    if (!html) { free(safeToken); return NULL; }
    snprintf(html, cap, "%s%s%s", pre, safeToken, post);
    free(safeToken);
    return html;
}

int secure_random_bytes(unsigned char* buf, size_t len) {
#ifdef _WIN32
    HCRYPTPROV prov;
    if (CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        BOOL ok = CryptGenRandom(prov, (DWORD)len, buf);
        CryptReleaseContext(prov, 0);
        return ok ? 1 : 0;
    }
    return 0;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t r = fread(buf, 1, len, f);
    fclose(f);
    return r == len;
#endif
}

char* generate_csrf_token() {
    unsigned char bytes[32];
    if (!secure_random_bytes(bytes, sizeof(bytes))) {
        // Fallback (not ideal): use time + rand
        srand((unsigned int)time(NULL));
        for (size_t i = 0; i < sizeof(bytes); i++) bytes[i] = (unsigned char)(rand() & 0xFF);
    }
    char* out = (char*)malloc(32 * 2 + 1);
    if (!out) return NULL;
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < 32; i++) {
        out[i * 2] = hex[(bytes[i] >> 4) & 0xF];
        out[i * 2 + 1] = hex[bytes[i] & 0xF];
    }
    out[64] = '\0';
    return out;
}

// Returns newly allocated string indicating redirect
char* post(const char* username, const char* message, const char* csrf_from_req, const char* session_csrf, DB* db) {
    const char* REDIR_CSRF = "REDIRECT:/?error=csrf";
    const char* REDIR_USER = "REDIRECT:/?error=bad_username";
    const char* REDIR_MSG  = "REDIRECT:/?error=bad_message";
    const char* REDIR_SRV  = "REDIRECT:/?error=server";
    const char* REDIR_OK   = "REDIRECT:/";

    if (!constant_time_equals(csrf_from_req ? csrf_from_req : "", session_csrf ? session_csrf : "")) {
        char* r = (char*)malloc(strlen(REDIR_CSRF) + 1);
        strcpy(r, REDIR_CSRF);
        return r;
    }
    if (!validate_username(username)) {
        char* r = (char*)malloc(strlen(REDIR_USER) + 1);
        strcpy(r, REDIR_USER);
        return r;
    }
    char* sanitized = sanitize_message(message);
    if (!sanitized) {
        char* r = (char*)malloc(strlen(REDIR_MSG) + 1);
        strcpy(r, REDIR_MSG);
        return r;
    }
    // Trim username
    char* u = strdup(username ? username : "");
    if (!u) { free(sanitized); char* r = (char*)malloc(strlen(REDIR_SRV) + 1); strcpy(r, REDIR_SRV); return r; }
    trim_inplace(u);

    int ok = insert_user_message_in_db(db, u, sanitized);
    free(sanitized);
    free(u);
    if (!ok) {
        char* r = (char*)malloc(strlen(REDIR_SRV) + 1);
        strcpy(r, REDIR_SRV);
        return r;
    }
    char* r = (char*)malloc(strlen(REDIR_OK) + 1);
    strcpy(r, REDIR_OK);
    return r;
}

int main(void) {
    DB db;
    db.count = 0;

    char* sessionToken = generate_csrf_token();
    if (!sessionToken) {
        fprintf(stderr, "Failed to generate CSRF token\n");
        return 1;
    }

    char* page = render_main_page(sessionToken);
    if (page) {
        printf("Rendered Main Page HTML:\n%s\n", page);
        free(page);
    } else {
        printf("Failed to render page\n");
    }

    // Test 1: Valid submission
    char* r1 = post("alice_01", "Hello world!", sessionToken, sessionToken, &db);
    printf("Test1: %s\n", r1); free(r1);

    // Test 2: Invalid username
    char* r2 = post("a", "short name", sessionToken, sessionToken, &db);
    printf("Test2: %s\n", r2); free(r2);

    // Test 3: Script content
    char* r3 = post("bob-2", "<script>alert(1)</script>", sessionToken, sessionToken, &db);
    printf("Test3: %s\n", r3); free(r3);

    // Test 4: Too long message
    char* longMsg = (char*)malloc(601);
    memset(longMsg, 'x', 600);
    longMsg[600] = '\0';
    char* r4 = post("charlie_3", longMsg, sessionToken, sessionToken, &db);
    printf("Test4: %s\n", r4); free(r4);
    free(longMsg);

    // Test 5: CSRF mismatch
    char* r5 = post("dave_4", "Valid message", "badtoken", sessionToken, &db);
    printf("Test5: %s\n", r5); free(r5);

    printf("DB size: %zu\n", db.count);
    for (size_t i = 0; i < db.count; i++) {
        printf("%s\n", db.records[i]);
    }

    free(sessionToken);
    return 0;
}