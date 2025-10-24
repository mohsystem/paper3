/* 
Chain-of-Through Steps:
1. Problem: Manage user sessions with create, validate, refresh, destroy, purge, and revoke by user.
2. Security: Use CSPRNG (/dev/urandom, getrandom, or arc4random_buf). Clamp TTL and validate userId. Avoid buffer overflows.
3. Secure coding: Bounds checks, safe string ops, input validation.
4. Code review: Checked for NULL checks, resource handling, and safe randomness.
5. Final secure code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__linux__)
#include <sys/random.h>
#endif

#if defined(__APPLE__)
#include <stdlib.h> // arc4random_buf
#endif

#define TOKEN_BYTES 32
#define TOKEN_HEX_LEN (TOKEN_BYTES*2)
#define MAX_USERID_LEN 128
#define MIN_TTL_SEC 1u
#define MAX_TTL_SEC (7u*24u*60u*60u)
#define MAX_SESSIONS 1024

typedef struct {
    int in_use;
    char id[TOKEN_HEX_LEN + 1];
    char userId[MAX_USERID_LEN + 1];
    time_t createdAt;
    time_t expiresAt;
    unsigned int ttlSec;
} Session;

static Session g_sessions[MAX_SESSIONS];

static time_t now_sec(void) {
    return time(NULL);
}

static unsigned int clamp_ttl(unsigned int ttl) {
    if (ttl < MIN_TTL_SEC) return MIN_TTL_SEC;
    if (ttl > MAX_TTL_SEC) return MAX_TTL_SEC;
    return ttl;
}

static int valid_user_id(const char* userId) {
    if (!userId) return 0;
    size_t len = strlen(userId);
    return len > 0 && len <= MAX_USERID_LEN;
}

static int get_secure_bytes(unsigned char* out, size_t len) {
#if defined(__linux__)
    ssize_t r = getrandom(out, len, 0);
    if (r == (ssize_t)len) return 1;
#endif
#if defined(__APPLE__)
    arc4random_buf(out, len);
    return 1;
#endif
    // Fallback to /dev/urandom
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t rd = fread(out, 1, len, f);
    fclose(f);
    return rd == len;
}

static void bytes_to_hex(const unsigned char* in, size_t inlen, char* out_hex /* size >= inlen*2+1 */) {
    static const char* hexdig = "0123456789abcdef";
    for (size_t i = 0; i < inlen; ++i) {
        unsigned char b = in[i];
        out_hex[i*2] = hexdig[(b >> 4) & 0xF];
        out_hex[i*2 + 1] = hexdig[b & 0xF];
    }
    out_hex[inlen*2] = '\0';
}

static int find_session_index_by_id(const char* token) {
    if (!token) return -1;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use && strcmp(g_sessions[i].id, token) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_free_slot(void) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!g_sessions[i].in_use) return i;
    }
    return -1;
}

// Returns malloc'd token string; caller must free. NULL on failure.
char* task123_create_session(const char* userId, unsigned int ttlSec) {
    if (!valid_user_id(userId)) {
        return NULL;
    }
    int slot = find_free_slot();
    if (slot < 0) {
        return NULL;
    }
    unsigned int ttl = clamp_ttl(ttlSec);

    unsigned char rnd[TOKEN_BYTES];
    if (!get_secure_bytes(rnd, sizeof(rnd))) {
        return NULL;
    }
    char token[TOKEN_HEX_LEN + 1];
    bytes_to_hex(rnd, sizeof(rnd), token);

    // Ensure uniqueness (unlikely to collide)
    if (find_session_index_by_id(token) >= 0) {
        return NULL; // Rare, but caller can retry
    }

    Session s;
    memset(&s, 0, sizeof(s));
    s.in_use = 1;
    strncpy(s.id, token, TOKEN_HEX_LEN);
    s.id[TOKEN_HEX_LEN] = '\0';
    strncpy(s.userId, userId, MAX_USERID_LEN);
    s.userId[MAX_USERID_LEN] = '\0';
    s.createdAt = now_sec();
    s.ttlSec = ttl;
    s.expiresAt = s.createdAt + (time_t)ttl;

    g_sessions[slot] = s;

    char* ret = (char*)malloc(strlen(token) + 1);
    if (!ret) {
        // Rollback
        g_sessions[slot].in_use = 0;
        return NULL;
    }
    strcpy(ret, token);
    return ret;
}

int task123_is_valid(const char* token) {
    int idx = find_session_index_by_id(token);
    if (idx < 0) return 0;
    time_t now = now_sec();
    if (now >= g_sessions[idx].expiresAt) {
        g_sessions[idx].in_use = 0;
        return 0;
    }
    return 1;
}

int task123_refresh_session(const char* token) {
    int idx = find_session_index_by_id(token);
    if (idx < 0) return 0;
    time_t now = now_sec();
    if (now >= g_sessions[idx].expiresAt) {
        g_sessions[idx].in_use = 0;
        return 0;
    }
    g_sessions[idx].expiresAt = now + (time_t)g_sessions[idx].ttlSec;
    return 1;
}

int task123_destroy_session(const char* token) {
    int idx = find_session_index_by_id(token);
    if (idx < 0) return 0;
    g_sessions[idx].in_use = 0;
    return 1;
}

int task123_purge_expired(void) {
    int removed = 0;
    time_t now = now_sec();
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use && now >= g_sessions[i].expiresAt) {
            g_sessions[i].in_use = 0;
            removed++;
        }
    }
    return removed;
}

int task123_revoke_user_sessions(const char* userId) {
    if (!valid_user_id(userId)) return 0;
    int removed = 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use && strcmp(g_sessions[i].userId, userId) == 0) {
            g_sessions[i].in_use = 0;
            removed++;
        }
    }
    return removed;
}

int task123_count_active_sessions(void) {
    task123_purge_expired();
    int count = 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use) count++;
    }
    return count;
}

// Test helper
int task123_debug_force_expire(const char* token) {
    int idx = find_session_index_by_id(token);
    if (idx < 0) return 0;
    g_sessions[idx].expiresAt = now_sec() - 1;
    return 1;
}

int main(void) {
    // Test 1: Create and validate
    char* s1 = task123_create_session("userA", 60);
    printf("T1 valid: %s\n", task123_is_valid(s1) ? "true" : "false");

    // Test 2: Create, force expire, validate false, purge
    char* s2 = task123_create_session("userB", 5);
    task123_debug_force_expire(s2);
    printf("T2 valid after force expire: %s\n", task123_is_valid(s2) ? "true" : "false");
    printf("T2 purge removed: %d\n", task123_purge_expired());

    // Test 3: Refresh
    printf("T3 refresh s1: %s\n", task123_refresh_session(s1) ? "true" : "false");
    printf("T3 s1 still valid: %s\n", task123_is_valid(s1) ? "true" : "false");

    // Test 4: Destroy
    printf("T4 destroy s1: %s\n", task123_destroy_session(s1) ? "true" : "false");
    printf("T4 s1 valid after destroy: %s\n", task123_is_valid(s1) ? "true" : "false");

    // Test 5: Revoke user sessions
    char* a = task123_create_session("userC", 10);
    char* b = task123_create_session("userC", 10);
    char* c = task123_create_session("userC", 10);
    (void)a; (void)b; (void)c;
    printf("T5 revoke userC count: %d\n", task123_revoke_user_sessions("userC"));
    printf("T5 active sessions: %d\n", task123_count_active_sessions());

    free(s1);
    free(s2);
    return 0;
}