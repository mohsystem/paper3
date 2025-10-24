#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SESSIONS 200
#define ID_LEN 32
#define USER_LEN 64

typedef struct {
    int used;
    char id[ID_LEN];
    char user[USER_LEN];
    long long createdAt;
    long long lastAccess;
    long long ttlMillis;
    int invalidated;
} Session;

static Session SESSIONS[MAX_SESSIONS];
static int SESSION_COUNTER = 0;

static int find_session_index(const char* sessionId) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (SESSIONS[i].used && strcmp(SESSIONS[i].id, sessionId) == 0) {
            return i;
        }
    }
    return -1;
}

static int expired(Session* s, long long now) {
    return (now - s->lastAccess) > s->ttlMillis;
}

const char* create_session(const char* user, long long nowMillis, long long ttlMillis) {
    int idx = -1;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!SESSIONS[i].used) { idx = i; break; }
    }
    if (idx == -1) return NULL;
    SESSION_COUNTER += 1;
    snprintf(SESSIONS[idx].id, ID_LEN, "S%d", SESSION_COUNTER);
    strncpy(SESSIONS[idx].user, user, USER_LEN-1);
    SESSIONS[idx].user[USER_LEN-1] = '\0';
    SESSIONS[idx].createdAt = nowMillis;
    SESSIONS[idx].lastAccess = nowMillis;
    SESSIONS[idx].ttlMillis = ttlMillis;
    SESSIONS[idx].invalidated = 0;
    SESSIONS[idx].used = 1;
    return SESSIONS[idx].id;
}

int is_active(const char* sessionId, long long nowMillis) {
    int idx = find_session_index(sessionId);
    if (idx < 0) return 0;
    Session* s = &SESSIONS[idx];
    if (s->invalidated) return 0;
    if (expired(s, nowMillis)) return 0;
    return 1;
}

int touch_session(const char* sessionId, long long nowMillis) {
    int idx = find_session_index(sessionId);
    if (idx < 0) return 0;
    Session* s = &SESSIONS[idx];
    if (s->invalidated || expired(s, nowMillis)) return 0;
    s->lastAccess = nowMillis;
    return 1;
}

int invalidate_session(const char* sessionId) {
    int idx = find_session_index(sessionId);
    if (idx < 0) return 0;
    Session* s = &SESSIONS[idx];
    if (s->invalidated) return 0;
    s->invalidated = 1;
    return 1;
}

int purge_expired(long long nowMillis) {
    int removed = 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!SESSIONS[i].used) continue;
        if (SESSIONS[i].invalidated || expired(&SESSIONS[i], nowMillis)) {
            SESSIONS[i].used = 0;
            removed++;
        }
    }
    return removed;
}

int list_active_sessions(const char* user, long long nowMillis, char* outBuf, int outBufSize) {
    outBuf[0] = '\0';
    int count = 0;
    int first = 1;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!SESSIONS[i].used) continue;
        if (strcmp(SESSIONS[i].user, user) == 0 && !SESSIONS[i].invalidated && !expired(&SESSIONS[i], nowMillis)) {
            if (!first) {
                strncat(outBuf, ", ", outBufSize - strlen(outBuf) - 1);
            }
            strncat(outBuf, SESSIONS[i].id, outBufSize - strlen(outBuf) - 1);
            first = 0;
            count++;
        }
    }
    return count;
}

int main() {
    char buf[256];

    // Test 1
    const char* s1 = create_session("alice", 1000, 30000);
    printf("Test1:isActive(s1@1000)=%s\n", is_active(s1, 1000) ? "true" : "false");
    list_active_sessions("alice", 1000, buf, sizeof(buf));
    printf("Test1:listActive(alice@1000)=[%s]\n", buf);

    // Test 2
    printf("Test2:touch(s1@20000)=%s\n", touch_session(s1, 20000) ? "true" : "false");
    printf("Test2:isActive(s1@20000)=%s\n", is_active(s1, 20000) ? "true" : "false");

    // Test 3
    const char* s2 = create_session("alice", 21000, 30000);
    list_active_sessions("alice", 21000, buf, sizeof(buf));
    printf("Test3:listActive(alice@21000)=[%s]\n", buf);

    // Test 4
    const char* sb = create_session("bob", 1000, 10000);
    printf("Test4:isActive(sb@12000)=%s\n", is_active(sb, 12000) ? "true" : "false");
    printf("Test4:purged@12000=%d\n", purge_expired(12000));
    list_active_sessions("bob", 12000, buf, sizeof(buf));
    printf("Test4:listActive(bob@12000)=[%s]\n", buf);

    // Test 5
    printf("Test5:invalidate(s1)=%s\n", invalidate_session(s1) ? "true" : "false");
    list_active_sessions("alice", 22000, buf, sizeof(buf));
    printf("Test5:listActive(alice@22000)=[%s]\n", buf);

    return 0;
}