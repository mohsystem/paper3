#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_VALUE_LEN 256

typedef struct {
    const char* username;
    const char* password;
    const char* session;
} Account;

static const Account ACCOUNTS[] = {
    {"mod_alice", "Str0ngP@ss!", "sess-AL1CE-123"},
    {"mod_bob",   "B0b$Secure",  "sess-BOB-456"}
};

static bool ct_equals(const char* a, const char* b) {
    if (a == NULL) a = "";
    if (b == NULL) b = "";
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t max = la > lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < max; i++) {
        unsigned char ac = i < la ? (unsigned char)a[i] : 0;
        unsigned char bc = i < lb ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(ac ^ bc);
    }
    return diff == 0;
}

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static bool url_decode_to_buf(const char* in, char* out, size_t out_size) {
    if (!in || !out || out_size == 0) return false;
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        char c = in[i];
        if (c == '+') {
            if (oi + 1 >= out_size) return false;
            out[oi++] = ' ';
        } else if (c == '%' && in[i+1] && in[i+2]) {
            int hi = hexVal(in[i+1]);
            int lo = hexVal(in[i+2]);
            if (hi < 0 || lo < 0) return false;
            if (oi + 1 >= out_size) return false;
            out[oi++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            if (oi + 1 >= out_size) return false;
            out[oi++] = c;
        }
        if (oi > MAX_VALUE_LEN) return false;
    }
    if (oi >= out_size) return false;
    out[oi] = '\0';
    return true;
}

static bool get_param_decoded(const char* url, const char* key, char* out, size_t out_size) {
    if (!url || !key || !out || out_size == 0) return false;
    const char* q = strchr(url, '?');
    if (!q) return false;
    size_t keylen = strlen(key);
    const char* p = q + 1;
    while (*p) {
        const char* amp = strchr(p, '&');
        size_t part_len = amp ? (size_t)(amp - p) : strlen(p);
        const char* eq = memchr(p, '=', part_len);
        size_t klen = eq ? (size_t)(eq - p) : part_len;
        size_t vlen = eq ? (part_len - klen - 1) : 0;

        if (klen == keylen && strncmp(p, key, keylen) == 0) {
            const char* vstart = eq ? (eq + 1) : "";
            char valbuf[MAX_VALUE_LEN + 1];
            if (vlen > 0) {
                char raw[ MAX_VALUE_LEN * 3 + 1 ]; // enough buffer for raw portion
                size_t copy_len = vlen < sizeof(raw) - 1 ? vlen : sizeof(raw) - 1;
                memcpy(raw, vstart, copy_len);
                raw[copy_len] = '\0';
                if (!url_decode_to_buf(raw, valbuf, sizeof(valbuf))) return false;
            } else {
                valbuf[0] = '\0';
            }
            if (strlen(valbuf) > MAX_VALUE_LEN) return false;
            strncpy(out, valbuf, out_size - 1);
            out[out_size - 1] = '\0';
            return true;
        }

        if (!amp) break;
        p = amp + 1;
    }
    return false;
}

bool auth_mod(const char* username, const char* password, const char* session) {
    int match = 0;
    for (size_t i = 0; i < sizeof(ACCOUNTS)/sizeof(ACCOUNTS[0]); i++) {
        int ueq = ct_equals(username, ACCOUNTS[i].username) ? 1 : 0;
        int peq = ct_equals(password, ACCOUNTS[i].password) ? 1 : 0;
        int seq = ct_equals(session, ACCOUNTS[i].session) ? 1 : 0;
        match |= (ueq & peq & seq);
    }
    return match == 1;
}

bool check_mod(const char* url) {
    if (!url) return false;
    // Check path
    const char* q = strchr(url, '?');
    size_t path_len = q ? (size_t)(q - url) : strlen(url);
    if (!(path_len == strlen("/check_mod") && strncmp(url, "/check_mod", path_len) == 0)) {
        return false;
    }
    char username[MAX_VALUE_LEN + 1] = {0};
    char password[MAX_VALUE_LEN + 1] = {0};
    char session[MAX_VALUE_LEN + 1] = {0};
    if (!get_param_decoded(url, "username", username, sizeof(username))) return false;
    if (!get_param_decoded(url, "password", password, sizeof(password))) return false;
    if (!get_param_decoded(url, "session", session, sizeof(session))) return false;
    return auth_mod(username, password, session);
}

int main(void) {
    const char* tests[5] = {
        "/check_mod?username=mod_alice&password=Str0ngP%40ss%21&session=sess-AL1CE-123",
        "/check_mod?username=mod_alice&password=wrong&session=sess-AL1CE-123",
        "/check_mod?username=mod_bob&password=B0b%24Secure&session=bad-session",
        "/check_mod?username=joe&password=anything&session=whatever",
        "/check_mod?username=mod_alice&password=Str0ngP%40ss%21"
    };
    for (int i = 0; i < 5; i++) {
        bool res = check_mod(tests[i]);
        printf("Test %d: %s\n", i + 1, res ? "true" : "false");
    }
    return 0;
}