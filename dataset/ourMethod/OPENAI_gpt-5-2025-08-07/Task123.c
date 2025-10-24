#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SESSIONS 128
#define MAX_USER_LEN 64
#define MIN_TOKEN_LEN 16
#define MAX_TOKEN_LEN 256

typedef struct {
    char token[MAX_TOKEN_LEN + 1];
    char user[MAX_USER_LEN + 1];
    long expires_at;  // epoch seconds
    long last_access; // epoch seconds
    int in_use;
} Session;

typedef struct {
    Session items[MAX_SESSIONS];
} SessionManager;

static long now_seconds(void) {
    return (long)time(NULL);
}

static int is_valid_token(const char* token) {
    if (token == NULL) return 0;
    size_t n = strlen(token);
    if (n < MIN_TOKEN_LEN || n > MAX_TOKEN_LEN) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)token[i];
        if (!(isalnum(c) || c == '-' || c == '_')) return 0;
    }
    return 1;
}

static int is_valid_user(const char* user) {
    if (user == NULL) return 0;
    size_t n = strlen(user);
    if (n < 1 || n > MAX_USER_LEN) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)user[i];
        if (!(isalnum(c) || c == '-' || c == '_' || c == '.')) return 0;
    }
    return 1;
}

static long clamp_ttl(long ttl) {
    if (ttl < 1) return 0;
    long max = 86400; // 1 day
    if (ttl > max) return max;
    return ttl;
}

static int constant_time_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) return 0;
    size_t la = strlen(a), lb = strlen(b);
    size_t lmax = la > lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < lmax; i++) {
        unsigned char xa = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char xb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(xa ^ xb);
    }
    return diff == 0;
}

static void init_manager(SessionManager* mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

static int find_index_by_token(SessionManager* mgr, const char* token) {
    if (!mgr || !token) return -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (mgr->items[i].in_use && constant_time_equals(mgr->items[i].token, token)) {
            return i;
        }
    }
    return -1;
}

static int add_session(SessionManager* mgr, const char* token, const char* user, long ttl_seconds) {
    if (!mgr || !is_valid_token(token) || !is_valid_user(user)) return 0;
    long ttl = clamp_ttl(ttl_seconds);
    if (ttl <= 0) return 0;
    if (find_index_by_token(mgr, token) >= 0) return 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!mgr->items[i].in_use) {
            mgr->items[i].in_use = 1;
            // Safe copy with bounds and termination
            snprintf(mgr->items[i].token, sizeof(mgr->items[i].token), "%s", token);
            snprintf(mgr->items[i].user, sizeof(mgr->items[i].user), "%s", user);
            long now = now_seconds();
            mgr->items[i].last_access = now;
            mgr->items[i].expires_at = now + ttl;
            return 1;
        }
    }
    return 0; // no space
}

static int validate_session(SessionManager* mgr, const char* token, char* out_user, size_t out_size) {
    if (!mgr || !is_valid_token(token) || !out_user || out_size == 0) return 0;
    int idx = find_index_by_token(mgr, token);
    if (idx < 0) return 0;
    long now = now_seconds();
    if (now > mgr->items[idx].expires_at) {
        mgr->items[idx].in_use = 0;
        return 0;
    }
    mgr->items[idx].last_access = now;
    snprintf(out_user, out_size, "%s", mgr->items[idx].user);
    return 1;
}

static int refresh_session(SessionManager* mgr, const char* old_token, const char* new_token, long ttl_seconds) {
    if (!mgr || !is_valid_token(old_token) || !is_valid_token(new_token)) return 0;
    if (constant_time_equals(old_token, new_token)) return 0;
    int old_idx = find_index_by_token(mgr, old_token);
    if (old_idx < 0) return 0;
    long now = now_seconds();
    if (now > mgr->items[old_idx].expires_at) {
        mgr->items[old_idx].in_use = 0;
        return 0;
    }
    if (find_index_by_token(mgr, new_token) >= 0) return 0;
    long ttl = clamp_ttl(ttl_seconds);
    // Find new slot
    int new_idx = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!mgr->items[i].in_use) { new_idx = i; break; }
    }
    if (new_idx < 0) return 0;
    mgr->items[new_idx].in_use = 1;
    snprintf(mgr->items[new_idx].token, sizeof(mgr->items[new_idx].token), "%s", new_token);
    snprintf(mgr->items[new_idx].user, sizeof(mgr->items[new_idx].user), "%s", mgr->items[old_idx].user);
    mgr->items[new_idx].last_access = now;
    mgr->items[new_idx].expires_at = now + ttl;
    mgr->items[old_idx].in_use = 0;
    return 1;
}

static int revoke_session(SessionManager* mgr, const char* token) {
    if (!mgr || !is_valid_token(token)) return 0;
    int idx = find_index_by_token(mgr, token);
    if (idx < 0) return 0;
    mgr->items[idx].in_use = 0;
    return 1;
}

static int prune_expired(SessionManager* mgr) {
    if (!mgr) return 0;
    int removed = 0;
    long now = now_seconds();
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (mgr->items[i].in_use && now > mgr->items[i].expires_at) {
            mgr->items[i].in_use = 0;
            removed++;
        }
    }
    return removed;
}

static int count_active(SessionManager* mgr) {
    if (!mgr) return 0;
    prune_expired(mgr);
    int cnt = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (mgr->items[i].in_use) cnt++;
    }
    return cnt;
}

int main(void) {
    SessionManager mgr;
    init_manager(&mgr);

    // Test 1: Create and validate (tokens are constants for testing; do not print tokens)
    const char* tok1 = "AAAAAAAAAAAAAAAAaaaa________----"; // 32 chars, allowed charset
    int add1 = add_session(&mgr, tok1, "alice", 5);
    char userbuf[65];
    int v1ok = validate_session(&mgr, tok1, userbuf, sizeof(userbuf));
    printf("T1 add=%s validUser=%s active=%d\n", add1 ? "true" : "false", v1ok ? userbuf : "", count_active(&mgr));

    // Test 2: Refresh token
    const char* tok2 = "BBBBBBBBBBBBBBBBbbbb________----";
    int ref = refresh_session(&mgr, tok1, tok2, 5);
    int vOldOk = validate_session(&mgr, tok1, userbuf, sizeof(userbuf));
    int vNewOk = validate_session(&mgr, tok2, userbuf, sizeof(userbuf));
    printf("T2 refresh=%s oldValid='%s' newValid='%s' active=%d\n",
           ref ? "true" : "false",
           vOldOk ? userbuf : "",
           vNewOk ? userbuf : "",
           count_active(&mgr));

    // Test 3: Revoke
    int rev = revoke_session(&mgr, tok2);
    int v3ok = validate_session(&mgr, tok2, userbuf, sizeof(userbuf));
    printf("T3 revoke=%s postRevokeValid='%s' active=%d\n",
           rev ? "true" : "false",
           v3ok ? userbuf : "",
           count_active(&mgr));

    // Test 4: Expiration
    const char* tok3 = "CCCCCCCCCCCCCCCCcccc________----";
    int add2 = add_session(&mgr, tok3, "bob", 1);
    usleep(1500000); // 1.5 seconds
    prune_expired(&mgr);
    int v4ok = validate_session(&mgr, tok3, userbuf, sizeof(userbuf));
    printf("T4 add=%s afterExpireValid='%s' active=%d\n",
           add2 ? "true" : "false",
           v4ok ? userbuf : "",
           count_active(&mgr));

    // Test 5: Invalid token
    int addBad = add_session(&mgr, "short", "charlie", 10);
    printf("T5 invalidTokenAdd=%s active=%d\n", addBad ? "true" : "false", count_active(&mgr));

    return 0;
}