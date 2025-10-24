#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_TOKENS 256
#define SESSION_MAX 64
#define TOKEN_BYTES 32
#define TOKEN_HEX_LEN (TOKEN_BYTES*2)

typedef struct {
    char session[SESSION_MAX];
    char token[TOKEN_HEX_LEN + 1];
    time_t expiry;
    int used;
    int in_use;
} TokenEntry;

static TokenEntry store[MAX_TOKENS];

static int secure_random(unsigned char* out, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, out + total, len - total);
        if (r <= 0) { close(fd); return 0; }
        total += (size_t)r;
    }
    close(fd);
    return 1;
}

static void hex_encode(const unsigned char* in, size_t len, char* out) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out[i*2] = hex[(in[i] >> 4) & 0xF];
        out[i*2 + 1] = hex[in[i] & 0xF];
    }
    out[len*2] = '\0';
}

static int is_state_changing(const char* method) {
    if (!method) return 0;
    char up[8] = {0};
    size_t i=0;
    for (; method[i] && i < sizeof(up)-1; ++i) {
        char c = method[i];
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        up[i] = c;
    }
    up[i] = '\0';
    return strcmp(up, "POST") == 0 || strcmp(up, "PUT") == 0 ||
           strcmp(up, "PATCH") == 0 || strcmp(up, "DELETE") == 0;
}

static int starts_with(const char* s, const char* prefix) {
    if (!s || !prefix) return 0;
    size_t lp = strlen(prefix);
    return strncmp(s, prefix, lp) == 0;
}

static int validate_origin_referer(const char* expected_origin, const char* origin, const char* referer) {
    if (!expected_origin || expected_origin[0] == '\0') return 1;
    if (origin && origin[0] != '\0') {
        return strcmp(expected_origin, origin) == 0;
    }
    if (referer && referer[0] != '\0') {
        return starts_with(referer, expected_origin);
    }
    return 0;
}

static void cleanup_expired() {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_TOKENS; ++i) {
        if (store[i].in_use && now > store[i].expiry) {
            store[i].in_use = 0;
        }
    }
}

int csrf_generate_token(const char* session_id, int ttl_seconds, char* out_token, size_t out_size) {
    if (!session_id || !out_token || out_size < (TOKEN_HEX_LEN + 1) || ttl_seconds <= 0) return 0;
    cleanup_expired();
    unsigned char rnd[TOKEN_BYTES];
    if (!secure_random(rnd, sizeof(rnd))) return 0;
    char hex[TOKEN_HEX_LEN + 1];
    hex_encode(rnd, sizeof(rnd), hex);

    // store
    int slot = -1;
    for (int i = 0; i < MAX_TOKENS; ++i) {
        if (!store[i].in_use) { slot = i; break; }
    }
    if (slot == -1) return 0;

    strncpy(store[slot].session, session_id, SESSION_MAX - 1);
    store[slot].session[SESSION_MAX - 1] = '\0';
    strncpy(store[slot].token, hex, TOKEN_HEX_LEN);
    store[slot].token[TOKEN_HEX_LEN] = '\0';
    store[slot].expiry = time(NULL) + ttl_seconds;
    store[slot].used = 0;
    store[slot].in_use = 1;

    strncpy(out_token, hex, out_size - 1);
    out_token[out_size - 1] = '\0';
    return 1;
}

int csrf_validate_request(const char* method,
                          const char* expected_origin,
                          const char* origin_header,
                          const char* referer_header,
                          const char* session_id,
                          const char* token) {
    if (!is_state_changing(method)) return 1;
    if (!validate_origin_referer(expected_origin, origin_header, referer_header)) return 0;
    if (!session_id || !token) return 0;
    cleanup_expired();
    time_t now = time(NULL);
    for (int i = 0; i < MAX_TOKENS; ++i) {
        if (store[i].in_use &&
            strcmp(store[i].session, session_id) == 0 &&
            strcmp(store[i].token, token) == 0) {
            if (store[i].used) return 0;
            if (now > store[i].expiry) { store[i].in_use = 0; return 0; }
            store[i].used = 1; // one-time use
            return 1;
        }
    }
    return 0;
}

int main(void) {
    const char* expected_origin = "https://example.com";
    char token[TOKEN_HEX_LEN + 1];
    const char* session = "sess-abc";

    int ok_gen = csrf_generate_token(session, 5, token, sizeof(token));
    if (!ok_gen) {
        printf("Generation failed\n");
        return 1;
    }

    int t1 = csrf_validate_request("POST", expected_origin, "https://example.com", NULL, session, token);
    int t2 = csrf_validate_request("POST", expected_origin, "https://evil.com", NULL, session, token);

    char tampered[TOKEN_HEX_LEN + 1];
    strncpy(tampered, token, sizeof(tampered));
    tampered[TOKEN_HEX_LEN - 1] = (tampered[TOKEN_HEX_LEN - 1] == 'a') ? 'b' : 'a';
    int t3 = csrf_validate_request("POST", expected_origin, "https://example.com", NULL, session, tampered);

    char token2[TOKEN_HEX_LEN + 1];
    csrf_generate_token("sess-exp", 1, token2, sizeof(token2));
    sleep(2);
    int t4 = csrf_validate_request("POST", expected_origin, "https://example.com", NULL, "sess-exp", token2);

    int t5 = csrf_validate_request("POST", expected_origin, "https://example.com", NULL, session, token);

    printf("T1_valid=%s\n", t1 ? "true" : "false");
    printf("T2_bad_origin=%s\n", t2 ? "true" : "false");
    printf("T3_tampered=%s\n", t3 ? "true" : "false");
    printf("T4_expired=%s\n", t4 ? "true" : "false");
    printf("T5_replay=%s\n", t5 ? "true" : "false");
    return 0;
}