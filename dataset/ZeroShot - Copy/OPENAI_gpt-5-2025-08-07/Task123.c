#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX_TTL_SECONDS (60 * 60 * 24 * 30) /* 30 days */
#define TOKEN_BYTES 32

typedef struct {
    char *token;
    char *userId;
    time_t expiry;
} Session;

typedef struct {
    Session *arr;
    size_t len;
    size_t cap;
} SessionManager;

static int secure_random_bytes(unsigned char *buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        if (r == 0) {
            close(fd);
            return 0;
        }
        off += (size_t)r;
    }
    close(fd);
    return 1;
}

static char *hex_encode(const unsigned char *buf, size_t len) {
    static const char *hex = "0123456789abcdef";
    char *out = (char *)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hex[(buf[i] >> 4) & 0xF];
        out[2*i + 1] = hex[buf[i] & 0xF];
    }
    out[len * 2] = '\0';
    return out;
}

static char *generate_token() {
    unsigned char buf[TOKEN_BYTES];
    if (!secure_random_bytes(buf, sizeof(buf))) return NULL;
    return hex_encode(buf, sizeof(buf));
}

static int ct_equal(const char *a, const char *b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    size_t max = la > lb ? la : lb;
    unsigned char acc = 0;
    for (size_t i = 0; i < max; ++i) {
        unsigned char av = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char bv = (i < lb) ? (unsigned char)b[i] : 0;
        acc |= (unsigned char)(av ^ bv);
    }
    return acc == 0;
}

static void sm_init(SessionManager *sm) {
    sm->arr = NULL;
    sm->len = 0;
    sm->cap = 0;
}

static void sm_free(SessionManager *sm) {
    if (!sm) return;
    for (size_t i = 0; i < sm->len; ++i) {
        free(sm->arr[i].token);
        free(sm->arr[i].userId);
    }
    free(sm->arr);
    sm->arr = NULL;
    sm->len = 0;
    sm->cap = 0;
}

static int sm_reserve(SessionManager *sm, size_t ncap) {
    if (sm->cap >= ncap) return 1;
    size_t newcap = sm->cap ? sm->cap : 8;
    while (newcap < ncap) newcap *= 2;
    Session *na = (Session *)realloc(sm->arr, newcap * sizeof(Session));
    if (!na) return 0;
    sm->arr = na;
    sm->cap = newcap;
    return 1;
}

static ssize_t sm_find_index(SessionManager *sm, const char *token) {
    if (!sm || !token) return -1;
    for (size_t i = 0; i < sm->len; ++i) {
        if (ct_equal(sm->arr[i].token, token)) {
            return (ssize_t)i;
        }
    }
    return -1;
}

static char *sm_create_session(SessionManager *sm, const char *userId, long ttlSeconds) {
    if (!sm || !userId || strlen(userId) == 0) return NULL;
    if (ttlSeconds <= 0) return NULL;
    if (ttlSeconds > MAX_TTL_SECONDS) ttlSeconds = MAX_TTL_SECONDS;

    char *token = NULL;
    int attempts = 0;
    do {
        free(token);
        token = generate_token();
        if (!token) return NULL;
        attempts++;
        if (attempts > 5) { free(token); return NULL; }
    } while (sm_find_index(sm, token) != -1);

    if (!sm_reserve(sm, sm->len + 1)) { free(token); return NULL; }

    Session s;
    s.token = strdup(token);
    s.userId = strdup(userId);
    if (!s.token || !s.userId) {
        free(s.token); free(s.userId); free(token);
        return NULL;
    }
    s.expiry = time(NULL) + ttlSeconds;
    sm->arr[sm->len++] = s;

    // Return a copy to caller to manage
    char *ret = strdup(token);
    free(token);
    return ret;
}

static char *sm_validate_session(SessionManager *sm, const char *token) {
    if (!sm || !token) return NULL;
    ssize_t idx = sm_find_index(sm, token);
    if (idx < 0) return NULL;
    time_t now = time(NULL);
    if (now > sm->arr[idx].expiry) {
        // remove expired
        free(sm->arr[idx].token);
        free(sm->arr[idx].userId);
        sm->arr[idx] = sm->arr[sm->len - 1];
        sm->len--;
        return NULL;
    }
    return strdup(sm->arr[idx].userId);
}

static int sm_refresh_session(SessionManager *sm, const char *token, long additionalTtlSeconds) {
    if (!sm || !token || additionalTtlSeconds <= 0) return 0;
    if (additionalTtlSeconds > MAX_TTL_SECONDS) additionalTtlSeconds = MAX_TTL_SECONDS;
    ssize_t idx = sm_find_index(sm, token);
    if (idx < 0) return 0;
    time_t now = time(NULL);
    if (now > sm->arr[idx].expiry) {
        // remove expired
        free(sm->arr[idx].token);
        free(sm->arr[idx].userId);
        sm->arr[idx] = sm->arr[sm->len - 1];
        sm->len--;
        return 0;
    }
    time_t newExpiry = sm->arr[idx].expiry + additionalTtlSeconds;
    time_t maxExpiry = now + MAX_TTL_SECONDS;
    if (newExpiry > maxExpiry) newExpiry = maxExpiry;
    sm->arr[idx].expiry = newExpiry;
    return 1;
}

static int sm_end_session(SessionManager *sm, const char *token) {
    if (!sm || !token) return 0;
    ssize_t idx = sm_find_index(sm, token);
    if (idx < 0) return 0;
    free(sm->arr[idx].token);
    free(sm->arr[idx].userId);
    sm->arr[idx] = sm->arr[sm->len - 1];
    sm->len--;
    return 1;
}

static int sm_cleanup_expired(SessionManager *sm) {
    if (!sm) return 0;
    int removed = 0;
    time_t now = time(NULL);
    for (size_t i = 0; i < sm->len; ) {
        if (now > sm->arr[i].expiry) {
            free(sm->arr[i].token);
            free(sm->arr[i].userId);
            sm->arr[i] = sm->arr[sm->len - 1];
            sm->len--;
            removed++;
        } else {
            i++;
        }
    }
    return removed;
}

int main(void) {
    SessionManager mgr;
    sm_init(&mgr);

    // Test 1: Create and validate
    char *t1 = sm_create_session(&mgr, "alice", 2);
    printf("T1 token: %s\n", t1 ? t1 : "(null)");
    char *v1 = sm_validate_session(&mgr, t1 ? t1 : "");
    printf("T1 validate: %s\n", v1 ? v1 : "(null)");
    free(v1);

    // Test 2: Validate invalid
    char *v2 = sm_validate_session(&mgr, "invalid_token");
    printf("T2 validate invalid: %s\n", v2 ? v2 : "(null)");
    free(v2);

    // Test 3: Refresh and validate
    int r = sm_refresh_session(&mgr, t1 ? t1 : "", 5);
    char *v3 = sm_validate_session(&mgr, t1 ? t1 : "");
    printf("T3 refreshed: %s, validate: %s\n", r ? "true" : "false", v3 ? v3 : "(null)");
    free(v3);

    // Test 4: End and validate
    int e = sm_end_session(&mgr, t1 ? t1 : "");
    char *v4 = sm_validate_session(&mgr, t1 ? t1 : "");
    printf("T4 ended: %s, validate after end: %s\n", e ? "true" : "false", v4 ? v4 : "(null)");
    free(v4);
    free(t1);

    // Test 5: Expiry and cleanup
    char *t2 = sm_create_session(&mgr, "bob", 1);
    char *v5a = sm_validate_session(&mgr, t2 ? t2 : "");
    printf("T5 token: %s, validate: %s\n", t2 ? t2 : "(null)", v5a ? v5a : "(null)");
    free(v5a);
    sleep(2);
    int cleaned = sm_cleanup_expired(&mgr);
    char *v5b = sm_validate_session(&mgr, t2 ? t2 : "");
    printf("T5 cleaned: %d, validate after expiry: %s\n", cleaned, v5b ? v5b : "(null)");
    free(v5b);
    free(t2);

    sm_free(&mgr);
    return 0;
}