#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SESSIONS 200
#define MAX_ATTRS 16
#define MAX_STR 256

typedef struct {
    char key[64];
    char value[256];
    int in_use;
} Attr;

typedef struct {
    char sessionId[64];
    char userId[64];
    long long expiryMs;
    Attr attrs[MAX_ATTRS];
    int in_use;
} Session;

static Session sessions[MAX_SESSIONS];
static int sessions_inited = 0;
static unsigned long long sid_counter = 0;

static long long now_ms() {
    struct timespec ts;
#if defined(_WIN32) || defined(_WIN64)
    // Fallback for Windows if clock_gettime is unavailable
    struct _timeb tb;
    _ftime(&tb);
    return (long long)tb.time * 1000LL + (long long)tb.millitm;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000LL + (long long)(ts.tv_nsec / 1000000LL);
#endif
}

static void ensure_init() {
    if (!sessions_inited) {
        memset(sessions, 0, sizeof(sessions));
        sessions_inited = 1;
    }
}

static int find_session_index(const char* session_id) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].in_use && strcmp(sessions[i].sessionId, session_id) == 0) return i;
    }
    return -1;
}

static int is_expired(Session* s) {
    return s->expiryMs < now_ms();
}

char* create_session(const char* user_id, long long ttl_ms) {
    ensure_init();
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!sessions[i].in_use) { slot = i; break; }
    }
    if (slot == -1) return NULL;
    sid_counter++;
    snprintf(sessions[slot].sessionId, sizeof(sessions[slot].sessionId), "S%llu_%lld", (unsigned long long)sid_counter, (long long)now_ms());
    snprintf(sessions[slot].userId, sizeof(sessions[slot].userId), "%s", user_id);
    sessions[slot].expiryMs = now_ms() + ttl_ms;
    for (int j = 0; j < MAX_ATTRS; j++) sessions[slot].attrs[j].in_use = 0;
    sessions[slot].in_use = 1;
    char* ret = (char*)malloc(strlen(sessions[slot].sessionId) + 1);
    if (!ret) return NULL;
    strcpy(ret, sessions[slot].sessionId);
    return ret;
}

int validate_session(const char* session_id) {
    ensure_init();
    int idx = find_session_index(session_id);
    if (idx < 0) return 0;
    if (is_expired(&sessions[idx])) {
        sessions[idx].in_use = 0;
        return 0;
    }
    return 1;
}

char* get_user(const char* session_id) {
    ensure_init();
    if (!validate_session(session_id)) return NULL;
    int idx = find_session_index(session_id);
    if (idx < 0) return NULL;
    char* ret = (char*)malloc(strlen(sessions[idx].userId) + 1);
    if (!ret) return NULL;
    strcpy(ret, sessions[idx].userId);
    return ret;
}

int set_attribute(const char* session_id, const char* key, const char* value) {
    ensure_init();
    if (!validate_session(session_id)) return 0;
    int idx = find_session_index(session_id);
    if (idx < 0) return 0;
    // Update if exists
    for (int i = 0; i < MAX_ATTRS; i++) {
        if (sessions[idx].attrs[i].in_use && strcmp(sessions[idx].attrs[i].key, key) == 0) {
            snprintf(sessions[idx].attrs[i].value, sizeof(sessions[idx].attrs[i].value), "%s", value);
            return 1;
        }
    }
    // Insert new
    for (int i = 0; i < MAX_ATTRS; i++) {
        if (!sessions[idx].attrs[i].in_use) {
            sessions[idx].attrs[i].in_use = 1;
            snprintf(sessions[idx].attrs[i].key, sizeof(sessions[idx].attrs[i].key), "%s", key);
            snprintf(sessions[idx].attrs[i].value, sizeof(sessions[idx].attrs[i].value), "%s", value);
            return 1;
        }
    }
    return 0;
}

char* get_attribute(const char* session_id, const char* key) {
    ensure_init();
    if (!validate_session(session_id)) return NULL;
    int idx = find_session_index(session_id);
    if (idx < 0) return NULL;
    for (int i = 0; i < MAX_ATTRS; i++) {
        if (sessions[idx].attrs[i].in_use && strcmp(sessions[idx].attrs[i].key, key) == 0) {
            char* ret = (char*)malloc(strlen(sessions[idx].attrs[i].value) + 1);
            if (!ret) return NULL;
            strcpy(ret, sessions[idx].attrs[i].value);
            return ret;
        }
    }
    return NULL;
}

int touch(const char* session_id, long long ttl_extend_ms) {
    ensure_init();
    if (!validate_session(session_id)) return 0;
    int idx = find_session_index(session_id);
    if (idx < 0) return 0;
    sessions[idx].expiryMs = now_ms() + ttl_extend_ms;
    return 1;
}

int invalidate_session(const char* session_id) {
    ensure_init();
    int idx = find_session_index(session_id);
    if (idx < 0) return 0;
    sessions[idx].in_use = 0;
    return 1;
}

int cleanup_expired() {
    ensure_init();
    int removed = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].in_use && is_expired(&sessions[i])) {
            sessions[i].in_use = 0;
            removed++;
        }
    }
    return removed;
}

int count_sessions() {
    ensure_init();
    int cnt = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) if (sessions[i].in_use) cnt++;
    return cnt;
}

int main() {
    char* sid1 = create_session("userA", 10000);
    printf("T1_valid=%d, user=%s, count=%d\n", validate_session(sid1), get_user(sid1), count_sessions());

    set_attribute(sid1, "role", "admin");
    char* role = get_attribute(sid1, "role");
    printf("T2_role=%s\n", role ? role : "null");
    if (role) free(role);

    int touched = touch(sid1, 10000);
    printf("T3_touched=%d, stillValid=%d\n", touched, validate_session(sid1));

    invalidate_session(sid1);
    printf("T4_validAfterInvalidate=%d, count=%d\n", validate_session(sid1), count_sessions());
    free(sid1);

    char* sid2 = create_session("userB", 0);
    char* sid3 = create_session("userC", -1000);
    int removed = cleanup_expired();
    printf("T5_removed=%d, finalCount=%d\n", removed, count_sessions());
    if (sid2) free(sid2);
    if (sid3) free(sid3);

    return 0;
}