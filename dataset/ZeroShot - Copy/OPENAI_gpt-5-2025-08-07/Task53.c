#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

#define MAX_SESSIONS 1024
#define SESSION_ID_BYTES 32
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2)
#define SESSION_ID_BUF_LEN (SESSION_ID_HEX_LEN + 1)
#define MAX_USER_LEN 64

typedef struct {
    int in_use;
    char id[SESSION_ID_BUF_LEN];
    char user[MAX_USER_LEN];
    time_t created_at;
    time_t expires_at;
    time_t last_accessed;
} Session;

typedef struct {
    Session entries[MAX_SESSIONS];
} SessionManager;

static int secure_random_bytes(uint8_t *buf, size_t len) {
#if defined(_WIN32) || defined(_WIN64)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    // Prefer getrandom if available, otherwise /dev/urandom
    // Portable approach: read from /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

static void bytes_to_hex(const uint8_t *in, size_t len, char *out_hex) {
    static const char *hex = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[i * 2] = hex[in[i] >> 4];
        out_hex[i * 2 + 1] = hex[in[i] & 0x0F];
    }
    out_hex[len * 2] = '\0';
}

static int ct_equals(const char *a, const char *b) {
    if (!a || !b) return 0;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t len = la < lb ? la : lb;
    unsigned char r = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < len; ++i) {
        r |= (unsigned char)(a[i] ^ b[i]);
    }
    return r == 0;
}

static time_t now_sec(void) {
    return time(NULL);
}

static void sm_init(SessionManager *sm) {
    if (!sm) return;
    memset(sm, 0, sizeof(SessionManager));
}

static int sm_generate_session_id(SessionManager *sm, char out_id[SESSION_ID_BUF_LEN]) {
    (void)sm;
    uint8_t buf[SESSION_ID_BYTES];
    for (int attempts = 0; attempts < 5; ++attempts) {
        if (!secure_random_bytes(buf, sizeof(buf))) return 0;
        bytes_to_hex(buf, sizeof(buf), out_id);
        // Ensure uniqueness
        int unique = 1;
        for (int i = 0; i < MAX_SESSIONS; ++i) {
            if (sm->entries[i].in_use && ct_equals(sm->entries[i].id, out_id)) {
                unique = 0;
                break;
            }
        }
        if (unique) return 1;
    }
    return 0;
}

static int sm_find_index_by_id(SessionManager *sm, const char *sid) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sm->entries[i].in_use && ct_equals(sm->entries[i].id, sid)) {
            return i;
        }
    }
    return -1;
}

static int sm_create_session(SessionManager *sm, const char *user_id, long ttl_seconds, char out_session_id[SESSION_ID_BUF_LEN]) {
    if (!sm || !user_id || ttl_seconds <= 0) return 0;
    int free_idx = -1;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!sm->entries[i].in_use) {
            free_idx = i; break;
        }
    }
    if (free_idx == -1) return 0;
    char sid[SESSION_ID_BUF_LEN];
    if (!sm_generate_session_id(sm, sid)) return 0;
    Session *s = &sm->entries[free_idx];
    memset(s, 0, sizeof(Session));
    s->in_use = 1;
    strncpy(s->id, sid, SESSION_ID_HEX_LEN);
    s->id[SESSION_ID_HEX_LEN] = '\0';
    strncpy(s->user, user_id, MAX_USER_LEN - 1);
    s->user[MAX_USER_LEN - 1] = '\0';
    s->created_at = now_sec();
    s->expires_at = s->created_at + ttl_seconds;
    s->last_accessed = s->created_at;
    strncpy(out_session_id, s->id, SESSION_ID_BUF_LEN);
    return 1;
}

static int sm_validate_session(SessionManager *sm, const char *session_id) {
    if (!sm || !session_id) return 0;
    int idx = sm_find_index_by_id(sm, session_id);
    if (idx < 0) return 0;
    Session *s = &sm->entries[idx];
    time_t now = now_sec();
    if (s->expires_at <= now) {
        s->in_use = 0;
        return 0;
    }
    s->last_accessed = now;
    return 1;
}

static int sm_get_user_for_session(SessionManager *sm, const char *session_id, char *out_user, size_t out_user_len) {
    if (!sm || !session_id || !out_user || out_user_len == 0) return 0;
    int idx = sm_find_index_by_id(sm, session_id);
    if (idx < 0) return 0;
    Session *s = &sm->entries[idx];
    time_t now = now_sec();
    if (s->expires_at <= now) {
        s->in_use = 0;
        return 0;
    }
    s->last_accessed = now;
    strncpy(out_user, s->user, out_user_len - 1);
    out_user[out_user_len - 1] = '\0';
    return 1;
}

static int sm_invalidate_session(SessionManager *sm, const char *session_id) {
    if (!sm || !session_id) return 0;
    int idx = sm_find_index_by_id(sm, session_id);
    if (idx < 0) return 0;
    sm->entries[idx].in_use = 0;
    return 1;
}

static int sm_invalidate_all_sessions_for_user(SessionManager *sm, const char *user_id) {
    if (!sm || !user_id) return 0;
    int count = 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sm->entries[i].in_use && strncmp(sm->entries[i].user, user_id, MAX_USER_LEN) == 0) {
            sm->entries[i].in_use = 0;
            count++;
        }
    }
    return count;
}

static int sm_touch_session(SessionManager *sm, const char *session_id, long extend_ttl_seconds) {
    if (!sm || !session_id || extend_ttl_seconds <= 0) return 0;
    int idx = sm_find_index_by_id(sm, session_id);
    if (idx < 0) return 0;
    Session *s = &sm->entries[idx];
    time_t now = now_sec();
    if (s->expires_at <= now) {
        s->in_use = 0;
        return 0;
    }
    time_t base = s->expires_at > now ? s->expires_at : now;
    s->expires_at = base + extend_ttl_seconds;
    s->last_accessed = now;
    return 1;
}

static int sm_rotate_session(SessionManager *sm, const char *session_id, char out_new_session_id[SESSION_ID_BUF_LEN]) {
    if (!sm || !session_id || !out_new_session_id) return 0;
    int idx = sm_find_index_by_id(sm, session_id);
    if (idx < 0) return 0;
    Session *s = &sm->entries[idx];
    time_t now = now_sec();
    if (s->expires_at <= now) {
        s->in_use = 0;
        return 0;
    }
    time_t remaining = s->expires_at - now;
    if (remaining <= 0) {
        s->in_use = 0;
        return 0;
    }
    char new_id[SESSION_ID_BUF_LEN];
    if (!sm_create_session(sm, s->user, (long)remaining, new_id)) return 0;
    // Invalidate old
    s->in_use = 0;
    strncpy(out_new_session_id, new_id, SESSION_ID_BUF_LEN);
    return 1;
}

static int sm_cleanup_expired_sessions(SessionManager *sm) {
    if (!sm) return 0;
    int count = 0;
    time_t now = now_sec();
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sm->entries[i].in_use && sm->entries[i].expires_at <= now) {
            sm->entries[i].in_use = 0;
            count++;
        }
    }
    return count;
}

static int sm_secure_compare_ids(const char *a, const char *b) {
    return ct_equals(a, b);
}

int main(void) {
    SessionManager sm;
    sm_init(&sm);

    // Test 1: Create and validate
    char s1[SESSION_ID_BUF_LEN];
    int ok = sm_create_session(&sm, "alice", 5, s1);
    printf("Test1 created=%d valid=%d\n", ok, sm_validate_session(&sm, s1));
    char user[128];
    printf("Test1 user=%s ok=%d\n", (sm_get_user_for_session(&sm, s1, user, sizeof(user)) ? user : ""), sm_get_user_for_session(&sm, s1, user, sizeof(user)));

    // Test 2: Touch/extend
    int touched = sm_touch_session(&sm, s1, 5);
    printf("Test2 touched=%d stillValid=%d\n", touched, sm_validate_session(&sm, s1));

    // Test 3: Rotate
    char rotated[SESSION_ID_BUF_LEN];
    int rotated_ok = sm_rotate_session(&sm, s1, rotated);
    printf("Test3 rotated_ok=%d oldValid=%d newValid=%d\n", rotated_ok, sm_validate_session(&sm, s1), sm_validate_session(&sm, rotated));

    // Test 4: Expiration
    char s2[SESSION_ID_BUF_LEN];
    sm_create_session(&sm, "bob", 1, s2);
#if defined(_WIN32) || defined(_WIN64)
    Sleep(1500);
#else
    struct timespec ts; ts.tv_sec = 1; ts.tv_nsec = 500000000; nanosleep(&ts, NULL);
#endif
    printf("Test4 expiredValid=%d\n", sm_validate_session(&sm, s2));

    // Test 5: Invalidate all for user
    char c1[SESSION_ID_BUF_LEN], c2[SESSION_ID_BUF_LEN];
    sm_create_session(&sm, "carol", 10, c1);
    sm_create_session(&sm, "carol", 10, c2);
    int invalidated = sm_invalidate_all_sessions_for_user(&sm, "carol");
    printf("Test5 invalidated=%d c1Valid=%d c2Valid=%d\n", invalidated, sm_validate_session(&sm, c1), sm_validate_session(&sm, c2));

    return 0;
}