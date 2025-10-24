#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif defined(__linux__)
#include <sys/random.h>
#include <unistd.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

typedef struct {
    char* token;
    char* user_id;
    char* client_ip;
    char* user_agent;
    uint64_t expires_at_ms;
    int in_use;
} Session;

#define MAX_SESSIONS 2048

typedef struct {
    Session sessions[MAX_SESSIONS];
    uint64_t default_ttl_ms;
} SessionManager;

static uint64_t now_ms() {
#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    // Convert from 100-ns since 1601 to ms since 1970
    uint64_t t = (uli.QuadPart - 116444736000000000ULL) / 10000ULL;
    return t;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
#endif
}

static int secure_random_bytes(unsigned char* buf, size_t len) {
#if defined(_WIN32)
    return BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#elif defined(__linux__)
    ssize_t n = getrandom(buf, len, 0);
    return n == (ssize_t)len;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, buf + off, len - off);
        if (n <= 0) { close(fd); return 0; }
        off += (size_t)n;
    }
    close(fd);
    return 1;
#endif
}

static char* hex_of(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hex[(data[i] >> 4) & 0xF];
        out[2*i + 1] = hex[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

static int ct_equals(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char r = 0;
    for (size_t i = 0; i < la; ++i) r |= ((unsigned char)a[i]) ^ ((unsigned char)b[i]);
    return r == 0;
}

static int bad(const char* s, size_t maxlen) {
    if (!s) return 1;
    size_t l = strlen(s);
    return l == 0 || l > maxlen;
}

static void sm_init(SessionManager* sm, uint64_t default_ttl_ms) {
    memset(sm, 0, sizeof(*sm));
    sm->default_ttl_ms = default_ttl_ms;
}

static void free_session(Session* s) {
    if (s->token) free(s->token);
    if (s->user_id) free(s->user_id);
    if (s->client_ip) free(s->client_ip);
    if (s->user_agent) free(s->user_agent);
    memset(s, 0, sizeof(*s));
}

static void sm_cleanup(SessionManager* sm) {
    uint64_t now = now_ms();
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sm->sessions[i].in_use && sm->sessions[i].expires_at_ms <= now) {
            free_session(&sm->sessions[i]);
            sm->sessions[i].in_use = 0;
        }
    }
}

static int sm_find_token(SessionManager* sm, const char* token) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sm->sessions[i].in_use && sm->sessions[i].token && strcmp(sm->sessions[i].token, token) == 0) {
            return i;
        }
    }
    return -1;
}

static int sm_alloc_slot(SessionManager* sm) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!sm->sessions[i].in_use) return i;
    }
    return -1;
}

static char* dupstr(const char* s) {
    size_t l = strlen(s);
    char* d = (char*)malloc(l + 1);
    if (!d) return NULL;
    memcpy(d, s, l + 1);
    return d;
}

char* create_session(SessionManager* sm, const char* user_id, const char* client_ip, const char* user_agent, uint64_t ttl_ms) {
    sm_cleanup(sm);
    if (bad(user_id, 128) || bad(client_ip, 64) || bad(user_agent, 256)) return NULL;
    if (ttl_ms == 0) ttl_ms = sm->default_ttl_ms;
    if (ttl_ms < 1000ULL) ttl_ms = 1000ULL;
    if (ttl_ms > 86400000ULL) ttl_ms = 86400000ULL;

    unsigned char rnd[32];
    if (!secure_random_bytes(rnd, sizeof(rnd))) return NULL;
    char* token = hex_of(rnd, sizeof(rnd));
    if (!token) return NULL;

    int idx = sm_alloc_slot(sm);
    if (idx < 0) { free(token); return NULL; }

    sm->sessions[idx].token = token;
    sm->sessions[idx].user_id = dupstr(user_id);
    sm->sessions[idx].client_ip = dupstr(client_ip);
    sm->sessions[idx].user_agent = dupstr(user_agent);
    sm->sessions[idx].expires_at_ms = now_ms() + ttl_ms;
    sm->sessions[idx].in_use = 1;
    return dupstr(token);
}

int validate_session(SessionManager* sm, const char* token, const char* client_ip, const char* user_agent) {
    sm_cleanup(sm);
    if (bad(token, 128) || bad(client_ip, 64) || bad(user_agent, 256)) return 0;
    int idx = sm_find_token(sm, token);
    if (idx < 0) return 0;
    Session* s = &sm->sessions[idx];
    if (s->expires_at_ms <= now_ms()) {
        free_session(s);
        s->in_use = 0;
        return 0;
    }
    return ct_equals(s->client_ip, client_ip) && ct_equals(s->user_agent, user_agent);
}

char* get_user_if_valid(SessionManager* sm, const char* token, const char* client_ip, const char* user_agent) {
    if (validate_session(sm, token, client_ip, user_agent)) {
        int idx = sm_find_token(sm, token);
        if (idx >= 0) return dupstr(sm->sessions[idx].user_id);
    }
    return NULL;
}

char* refresh_session(SessionManager* sm, const char* token, const char* client_ip, const char* user_agent) {
    sm_cleanup(sm);
    if (!validate_session(sm, token, client_ip, user_agent)) return NULL;
    int idx = sm_find_token(sm, token);
    if (idx < 0) return NULL;

    Session* old = &sm->sessions[idx];
    char* new_token = create_session(sm, old->user_id, old->client_ip, old->user_agent, sm->default_ttl_ms);
    // Terminate old
    free_session(old);
    old->in_use = 0;
    return new_token;
}

int terminate_session(SessionManager* sm, const char* token) {
    int idx = sm_find_token(sm, token);
    if (idx < 0) return 0;
    free_session(&sm->sessions[idx]);
    sm->sessions[idx].in_use = 0;
    return 1;
}

int terminate_all_for_user(SessionManager* sm, const char* user_id) {
    if (bad(user_id, 128)) return 0;
    int count = 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sm->sessions[i].in_use && sm->sessions[i].user_id && strcmp(sm->sessions[i].user_id, user_id) == 0) {
            free_session(&sm->sessions[i]);
            sm->sessions[i].in_use = 0;
            count++;
        }
    }
    return count;
}

int main() {
    SessionManager sm;
    sm_init(&sm, 1800000ULL);

    const char* ip = "203.0.113.10";
    const char* ua = "ExampleBrowser/1.0";
    const char* bad_ip = "198.51.100.5";

    char* tok1 = create_session(&sm, "alice", ip, ua, 0);
    printf("T1 valid: %s\n", validate_session(&sm, tok1, ip, ua) ? "true" : "false");

    printf("T2 wrong IP valid: %s\n", validate_session(&sm, tok1, bad_ip, ua) ? "true" : "false");

    char* tok2 = refresh_session(&sm, tok1, ip, ua);
    printf("T3 old valid: %s\n", validate_session(&sm, tok1, ip, ua) ? "true" : "false");
    printf("T3 new valid: %s\n", validate_session(&sm, tok2, ip, ua) ? "true" : "false");

    char* tok3 = create_session(&sm, "bob", ip, ua, 1000ULL);
#if defined(_WIN32)
    Sleep(1200);
#else
    usleep(1200 * 1000);
#endif
    printf("T4 expired valid: %s\n", validate_session(&sm, tok3, ip, ua) ? "true" : "false");

    char* tok4 = create_session(&sm, "carol", ip, ua, 0);
    printf("T5 before terminate: %s\n", validate_session(&sm, tok4, ip, ua) ? "true" : "false");
    terminate_session(&sm, tok4);
    printf("T5 after terminate: %s\n", validate_session(&sm, tok4, ip, ua) ? "true" : "false");

    free(tok1);
    free(tok2);
    free(tok3);
    free(tok4);

    return 0;
}