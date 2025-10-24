#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <unistd.h>
#endif

/*
 Chain-of-Through summary:
 1) Implement session manager: create, validate, get, refresh, invalidate, prune.
 2) Security: OS CSPRNG, base64url tokens, expiration, rotate tokens, constant-time compare helper.
 3) Validate inputs, no secret prints beyond tests, careful memory handling.
 4) Reviewed for leaks and errors; simple dynamic array for storage.
 5) Provide 5 test cases.
*/

typedef struct {
    char* userId;
    int64_t createdAtMs;
    int64_t expiresAtMs;
    char* token;
} Session;

typedef struct {
    Session* arr;
    size_t size;
    size_t cap;
    int64_t ttlMs;
    size_t tokenBytes;
} SessionManager;

typedef struct {
    char* userId;
    int64_t expiresAtMs;
} SessionInfo;

static int64_t now_ms() {
#if defined(_WIN32)
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (int64_t)(counter.QuadPart * 1000 / freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

static int secure_random_bytes(unsigned char* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t readTotal = 0;
    while (readTotal < len) {
        size_t r = fread(buf + readTotal, 1, len - readTotal, f);
        if (r == 0) break;
        readTotal += r;
    }
    fclose(f);
    return readTotal == len;
#endif
}

static char* base64url_encode(const unsigned char* data, size_t len) {
    static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t outLen = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(outLen + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i < len) {
        uint32_t octet_a = i < len ? data[i++] : 0;
        uint32_t octet_b = i < len ? data[i++] : 0;
        uint32_t octet_c = i < len ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        out[j++] = b64[(triple >> 18) & 0x3F];
        out[j++] = b64[(triple >> 12) & 0x3F];
        out[j++] = (i > len + 1) ? '=' : b64[(triple >> 6) & 0x3F];
        out[j++] = (i > len) ? '=' : b64[triple & 0x3F];
    }
    out[j] = '\0';
    // Convert to URL-safe and strip padding
    for (size_t k = 0; k < j; k++) {
        if (out[k] == '+') out[k] = '-';
        else if (out[k] == '/') out[k] = '_';
    }
    // remove '='
    size_t k = j;
    while (k > 0 && out[k - 1] == '=') k--;
    out[k] = '\0';
    return out;
}

static int ct_strcmp(const char* a, const char* b) {
    if (!a || !b) return 1;
    size_t la = strlen(a), lb = strlen(b);
    size_t n = la > lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < n; i++) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(ca ^ cb);
    }
    return diff; // 0 if equal
}

static void sm_init(SessionManager* sm, int64_t ttlMs, size_t tokenBytes) {
    sm->arr = NULL;
    sm->size = 0;
    sm->cap = 0;
    sm->ttlMs = ttlMs;
    sm->tokenBytes = tokenBytes;
}

static void sm_free(SessionManager* sm) {
    if (!sm) return;
    for (size_t i = 0; i < sm->size; i++) {
        free(sm->arr[i].userId);
        free(sm->arr[i].token);
    }
    free(sm->arr);
    sm->arr = NULL;
    sm->size = 0;
    sm->cap = 0;
}

static int sm_grow(SessionManager* sm) {
    size_t newCap = sm->cap == 0 ? 8 : sm->cap * 2;
    Session* na = (Session*)realloc(sm->arr, newCap * sizeof(Session));
    if (!na) return 0;
    sm->arr = na;
    sm->cap = newCap;
    return 1;
}

static char* generate_token(size_t tokenBytes) {
    unsigned char* buf = (unsigned char*)malloc(tokenBytes);
    if (!buf) return NULL;
    if (!secure_random_bytes(buf, tokenBytes)) {
        free(buf);
        return NULL;
    }
    char* tok = base64url_encode(buf, tokenBytes);
    free(buf);
    return tok;
}

static char* sm_create_session(SessionManager* sm, const char* userId) {
    if (!sm || !userId || userId[0] == '\0' || sm->ttlMs <= 0 || sm->tokenBytes < 16) return NULL;
    if (sm->size == sm->cap && !sm_grow(sm)) return NULL;
    char* token = generate_token(sm->tokenBytes);
    if (!token) return NULL;
    char* uid = strdup(userId);
    if (!uid) { free(token); return NULL; }
    int64_t now = now_ms();
    sm->arr[sm->size].userId = uid;
    sm->arr[sm->size].token = token;
    sm->arr[sm->size].createdAtMs = now;
    sm->arr[sm->size].expiresAtMs = now + sm->ttlMs;
    sm->size += 1;
    return strdup(token);
}

static int sm_find_index(SessionManager* sm, const char* token) {
    if (!sm || !token) return -1;
    for (size_t i = 0; i < sm->size; i++) {
        if (ct_strcmp(sm->arr[i].token, token) == 0) return (int)i;
    }
    return -1;
}

static int sm_validate_session(SessionManager* sm, const char* token) {
    if (!sm || !token) return 0;
    int idx = sm_find_index(sm, token);
    if (idx < 0) return 0;
    int64_t now = now_ms();
    if (now > sm->arr[idx].expiresAtMs) {
        // remove expired
        free(sm->arr[idx].userId);
        free(sm->arr[idx].token);
        sm->arr[idx] = sm->arr[sm->size - 1];
        sm->size -= 1;
        return 0;
    }
    return 1;
}

static SessionInfo* sm_get_session_info(SessionManager* sm, const char* token, SessionInfo* out) {
    if (!out) return NULL;
    if (!sm_validate_session(sm, token)) return NULL;
    int idx = sm_find_index(sm, token);
    if (idx < 0) return NULL;
    out->userId = sm->arr[idx].userId;
    out->expiresAtMs = sm->arr[idx].expiresAtMs;
    return out;
}

static int sm_invalidate_session(SessionManager* sm, const char* token) {
    if (!sm || !token) return 0;
    int idx = sm_find_index(sm, token);
    if (idx < 0) return 0;
    free(sm->arr[idx].userId);
    free(sm->arr[idx].token);
    sm->arr[idx] = sm->arr[sm->size - 1];
    sm->size -= 1;
    return 1;
}

static char* sm_refresh_session(SessionManager* sm, const char* token) {
    if (!sm_validate_session(sm, token)) return NULL;
    int idx = sm_find_index(sm, token);
    if (idx < 0) return NULL;
    char* newTok = generate_token(sm->tokenBytes);
    if (!newTok) return NULL;
    int64_t now = now_ms();
    // Create new session with same user, extend expiry, remove old
    char* userCopy = strdup(sm->arr[idx].userId);
    if (!userCopy) { free(newTok); return NULL; }

    // Remove old
    free(sm->arr[idx].userId);
    free(sm->arr[idx].token);
    sm->arr[idx] = sm->arr[sm->size - 1];
    sm->size -= 1;

    // Insert new
    if (sm->size == sm->cap && !sm_grow(sm)) { free(newTok); free(userCopy); return NULL; }
    sm->arr[sm->size].userId = userCopy;
    sm->arr[sm->size].token = newTok;
    sm->arr[sm->size].createdAtMs = now; // rotation time
    sm->arr[sm->size].expiresAtMs = now + sm->ttlMs;
    sm->size += 1;

    return strdup(newTok);
}

static int sm_prune_expired(SessionManager* sm) {
    if (!sm) return 0;
    int removed = 0;
    int64_t now = now_ms();
    for (size_t i = 0; i < sm->size; ) {
        if (now > sm->arr[i].expiresAtMs) {
            free(sm->arr[i].userId);
            free(sm->arr[i].token);
            sm->arr[i] = sm->arr[sm->size - 1];
            sm->size -= 1;
            removed++;
        } else {
            i++;
        }
    }
    return removed;
}

int main(void) {
    SessionManager mgr;
    sm_init(&mgr, 5000, 32);

    // 1) Create and validate
    char* t1 = sm_create_session(&mgr, "alice");
    printf("Test1 valid: %s\n", sm_validate_session(&mgr, t1) ? "true" : "false");

    // 2) Get session info
    SessionInfo info;
    SessionInfo* pinfo = sm_get_session_info(&mgr, t1, &info);
    if (pinfo) {
        printf("Test2 info: SessionInfo(userId='%s', expiresAtMs=%lld)\n", pinfo->userId, (long long)pinfo->expiresAtMs);
    } else {
        printf("Test2 info: null\n");
    }

    // 3) Refresh session
    char* t1New = sm_refresh_session(&mgr, t1);
    printf("Test3 new valid: %s, old valid: %s\n",
           sm_validate_session(&mgr, t1New) ? "true" : "false",
           sm_validate_session(&mgr, t1) ? "true" : "false");

    // 4) Invalidate session
    int inv = sm_invalidate_session(&mgr, t1New);
    printf("Test4 invalidated: %s, validate after: %s\n", inv ? "true" : "false",
           sm_validate_session(&mgr, t1New) ? "true" : "false");

    free(t1);
    free(t1New);

    // 5) Expiration test with short TTL
    SessionManager shortMgr;
    sm_init(&shortMgr, 100, 32);
    char* t2 = sm_create_session(&shortMgr, "bob");
    printf("Test5 before expiry valid: %s\n", sm_validate_session(&shortMgr, t2) ? "true" : "false");
#if defined(_WIN32)
    Sleep(200);
#else
    struct timespec req; req.tv_sec = 0; req.tv_nsec = 200000000; nanosleep(&req, NULL);
#endif
    sm_prune_expired(&shortMgr);
    printf("Test5 after expiry valid: %s\n", sm_validate_session(&shortMgr, t2) ? "true" : "false");

    free(t2);
    sm_free(&mgr);
    sm_free(&shortMgr);
    return 0;
}