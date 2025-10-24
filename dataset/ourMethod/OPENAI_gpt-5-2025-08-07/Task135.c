#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#if defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h> // arc4random_buf
#elif defined(_MSC_VER)
#include <stdlib.h> // rand_s
#endif

#define MAX_TOKENS 512
#define MAX_SESSION_LEN 64
#define MAX_TOKEN_LEN 128
#define TTL_SECONDS 600

typedef struct {
    char session[MAX_SESSION_LEN + 1];
    char *token; // dynamically allocated
    long long expiry;
    int in_use;
} TokenEntry;

static TokenEntry store[MAX_TOKENS];

static long long now_seconds(void) {
    return (long long)time(NULL);
}

static int is_valid_session_id(const char *s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len < 1 || len > MAX_SESSION_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-' || c == '_')) {
            return 0;
        }
    }
    return 1;
}

static int secure_random_bytes(uint8_t *buf, size_t n) {
#if defined(__linux__)
    size_t off = 0;
    while (off < n) {
        ssize_t r = getrandom(buf + off, n - off, 0);
        if (r < 0) return 0;
        off += (size_t)r;
    }
    return 1;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    arc4random_buf(buf, n);
    return 1;
#elif defined(_MSC_VER)
    size_t off = 0;
    while (off < n) {
        unsigned int v;
        if (rand_s(&v) != 0) return 0;
        size_t copy = (n - off) < 4 ? (n - off) : 4;
        for (size_t i = 0; i < copy; i++) {
            buf[off + i] = (uint8_t)((v >> (i * 8)) & 0xFFu);
        }
        off += copy;
    }
    return 1;
#else
    return 0; // No secure RNG available
#endif
}

static char *base64url_encode(const uint8_t *data, size_t len) {
    static const char *table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t out_len = (len / 3) * 4 + ((len % 3) == 0 ? 0 : ((len % 3) == 1 ? 2 : 3));
    char *out = (char *)malloc(out_len + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        uint32_t triple = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | ((uint32_t)data[i + 2]);
        out[j++] = table[(triple >> 18) & 0x3F];
        out[j++] = table[(triple >> 12) & 0x3F];
        out[j++] = table[(triple >> 6) & 0x3F];
        out[j++] = table[triple & 0x3F];
        i += 3;
    }
    size_t rem = len - i;
    if (rem == 1) {
        uint32_t triple = ((uint32_t)data[i] << 16);
        out[j++] = table[(triple >> 18) & 0x3F];
        out[j++] = table[(triple >> 12) & 0x3F];
    } else if (rem == 2) {
        uint32_t triple = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8);
        out[j++] = table[(triple >> 18) & 0x3F];
        out[j++] = table[(triple >> 12) & 0x3F];
        out[j++] = table[(triple >> 6) & 0x3F];
    }
    out[j] = '\0';
    return out;
}

static int constant_time_equals(const char *a, const char *b) {
    if (a == NULL || b == NULL) return 0;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t max = (la > lb) ? la : lb;
    unsigned int diff = (unsigned int)(la ^ lb);
    for (size_t i = 0; i < max; i++) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ca ^ cb);
    }
    return diff == 0;
}

static void purge_expired(void) {
    long long now = now_seconds();
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (store[i].in_use && store[i].expiry <= now) {
            store[i].in_use = 0;
            if (store[i].token) {
                free(store[i].token);
                store[i].token = NULL;
            }
            store[i].session[0] = '\0';
            store[i].expiry = 0;
        }
    }
}

static char *generate_token(const char *session_id) {
    if (!is_valid_session_id(session_id)) {
        return NULL;
    }
    uint8_t rnd[32];
    if (!secure_random_bytes(rnd, sizeof(rnd))) {
        return NULL;
    }
    char *tok = base64url_encode(rnd, sizeof(rnd));
    if (!tok) return NULL;

    // Store it
    purge_expired();
    int placed = 0;
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (!store[i].in_use) {
            store[i].in_use = 1;
            store[i].expiry = now_seconds() + TTL_SECONDS;
            snprintf(store[i].session, sizeof(store[i].session), "%s", session_id);
            store[i].token = strdup(tok);
            if (!store[i].token) {
                store[i].in_use = 0;
                store[i].session[0] = '\0';
                free(tok);
                return NULL;
            }
            placed = 1;
            break;
        }
    }
    if (!placed) {
        free(tok);
        return NULL;
    }
    // Return a duplicate for caller usage
    char *ret = strdup(tok);
    free(tok);
    return ret;
}

static int validate_token(const char *session_id, const char *token) {
    if (!is_valid_session_id(session_id)) return 0;
    if (token == NULL) return 0;
    size_t tlen = strlen(token);
    if (tlen < 16 || tlen > MAX_TOKEN_LEN) return 0;

    long long now = now_seconds();
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (store[i].in_use) {
            if (store[i].expiry <= now) {
                // Expired; clean up
                store[i].in_use = 0;
                if (store[i].token) {
                    free(store[i].token);
                    store[i].token = NULL;
                }
                store[i].session[0] = '\0';
                store[i].expiry = 0;
                continue;
            }
            if (strncmp(store[i].session, session_id, MAX_SESSION_LEN) == 0) {
                if (constant_time_equals(store[i].token, token)) {
                    // One-time use
                    store[i].in_use = 0;
                    if (store[i].token) {
                        free(store[i].token);
                        store[i].token = NULL;
                    }
                    store[i].session[0] = '\0';
                    store[i].expiry = 0;
                    purge_expired();
                    return 1;
                }
            }
        }
    }
    purge_expired();
    return 0;
}

static int expire_token_for_test(const char *session_id, const char *token) {
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (store[i].in_use &&
            strncmp(store[i].session, session_id, MAX_SESSION_LEN) == 0 &&
            constant_time_equals(store[i].token, token)) {
            store[i].expiry = now_seconds() - 1;
            return 1;
        }
    }
    return 0;
}

int main(void) {
    // Test 1: Valid token for session
    const char *s1 = "sessA";
    char *tok1 = generate_token(s1);
    int t1 = validate_token(s1, tok1);
    printf("%s\n", t1 ? "true" : "false");

    // Test 2: Wrong session usage
    const char *s2 = "sessB";
    int t2 = validate_token(s2, tok1);
    printf("%s\n", t2 ? "true" : "false");

    // Test 3: Tampered token fails
    char *tok3 = generate_token(s1);
    if (!tok3) {
        printf("false\n");
    } else {
        size_t l3 = strlen(tok3);
        char *tampered = (char *)malloc(l3 + 1);
        if (!tampered) {
            printf("false\n");
        } else {
            strcpy(tampered, tok3);
            tampered[0] = (tampered[0] == 'A') ? 'B' : 'A';
            int t3 = validate_token(s1, tampered);
            printf("%s\n", t3 ? "true" : "false");
            free(tampered);
        }
        free(tok3);
    }

    // Test 4: Expired token fails
    char *tok4 = generate_token("sessX");
    if (tok4) {
        expire_token_for_test("sessX", tok4);
        int t4 = validate_token("sessX", tok4);
        printf("%s\n", t4 ? "true" : "false");
        free(tok4);
    } else {
        printf("false\n");
    }

    // Test 5: Replay detection (second use fails)
    char *tok5 = generate_token("sessY");
    int first = tok5 ? validate_token("sessY", tok5) : 0;
    int second = tok5 ? validate_token("sessY", tok5) : 0;
    int t5 = first && !second;
    printf("%s\n", t5 ? "true" : "false");
    if (tok5) free(tok5);

    if (tok1) free(tok1);
    return 0;
}