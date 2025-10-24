#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

typedef struct TokenRecord {
    char *session_id;
    char *token;
    uint64_t expiry_ms;
    struct TokenRecord *next;
} TokenRecord;

typedef struct {
    TokenRecord *head;
    uint64_t ttl_ms;
} CsrfService;

static uint64_t now_ms(void) {
#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    // Convert from 100-nanoseconds since Jan 1, 1601 to milliseconds
    uint64_t ms = (uli.QuadPart - 116444736000000000ULL) / 10000ULL;
    return ms;
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
#endif
}

static int secure_random_bytes(unsigned char *buf, size_t len) {
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return 0;
    }
    BOOL ok = CryptGenRandom(hProv, (DWORD)len, buf);
    CryptReleaseContext(hProv, 0);
    return ok ? 1 : 0;
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t readn = fread(buf, 1, len, f);
    fclose(f);
    return readn == len ? 1 : 0;
#endif
}

static char *bytes_to_hex(const unsigned char *buf, size_t len) {
    static const char *hex = "0123456789abcdef";
    size_t outlen = len * 2;
    char *out = (char *)malloc(outlen + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i]     = hex[(buf[i] >> 4) & 0xF];
        out[2*i + 1] = hex[buf[i] & 0xF];
    }
    out[outlen] = '\0';
    return out;
}

static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *d = (char *)malloc(n + 1);
    if (!d) return NULL;
    memcpy(d, s, n + 1);
    return d;
}

static int constant_time_equals(const char *a, const char *b) {
    if (!a || !b) return 0;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    if (la != lb) return 0;
    unsigned char diff = 0;
    for (size_t i = 0; i < la; ++i) {
        diff |= (unsigned char)(a[i] ^ b[i]);
    }
    return diff == 0;
}

static TokenRecord *find_record(CsrfService *svc, const char *session_id) {
    TokenRecord *cur = svc->head;
    while (cur) {
        if (strcmp(cur->session_id, session_id) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

static void purge_expired(CsrfService *svc) {
    uint64_t now = now_ms();
    TokenRecord *cur = svc->head;
    TokenRecord *prev = NULL;
    while (cur) {
        if (now > cur->expiry_ms) {
            TokenRecord *to_free = cur;
            if (prev) prev->next = cur->next;
            else svc->head = cur->next;
            cur = cur->next;
            free(to_free->session_id);
            free(to_free->token);
            free(to_free);
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
}

static void csrf_init(CsrfService *svc, uint64_t ttl_ms) {
    svc->head = NULL;
    svc->ttl_ms = ttl_ms;
}

static void csrf_free(CsrfService *svc) {
    TokenRecord *cur = svc->head;
    while (cur) {
        TokenRecord *n = cur->next;
        free(cur->session_id);
        free(cur->token);
        free(cur);
        cur = n;
    }
    svc->head = NULL;
}

static char *csrf_issue_token(CsrfService *svc, const char *session_id) {
    if (!session_id) return NULL;
    unsigned char buf[32];
    if (!secure_random_bytes(buf, sizeof buf)) return NULL;
    char *token = bytes_to_hex(buf, sizeof buf);
    if (!token) return NULL;

    TokenRecord *rec = find_record(svc, session_id);
    uint64_t expiry = now_ms() + svc->ttl_ms;
    if (rec) {
        free(rec->token);
        rec->token = token;
        rec->expiry_ms = expiry;
    } else {
        TokenRecord *nr = (TokenRecord *)calloc(1, sizeof(TokenRecord));
        if (!nr) {
            free(token);
            return NULL;
        }
        nr->session_id = safe_strdup(session_id);
        nr->token = token;
        nr->expiry_ms = expiry;
        nr->next = svc->head;
        svc->head = nr;
    }
    return safe_strdup(token);
}

static int csrf_validate_token(CsrfService *svc, const char *session_id, const char *provided_token) {
    if (!session_id || !provided_token) return 0;
    purge_expired(svc);
    TokenRecord *rec = find_record(svc, session_id);
    if (!rec) return 0;
    if (now_ms() > rec->expiry_ms) return 0;
    int ok = constant_time_equals(rec->token, provided_token);
    if (ok) {
        // rotate token
        unsigned char buf[32];
        if (!secure_random_bytes(buf, sizeof buf)) return 0;
        char *newtok = bytes_to_hex(buf, sizeof buf);
        if (!newtok) return 0;
        free(rec->token);
        rec->token = newtok;
        rec->expiry_ms = now_ms() + svc->ttl_ms;
    }
    return ok;
}

int main(void) {
    CsrfService svc;
    csrf_init(&svc, 200); // 200 ms TTL

    // Test 1: Valid token
    const char *s1 = "sessA";
    char *t1 = csrf_issue_token(&svc, s1);
    int r1 = csrf_validate_token(&svc, s1, t1);

    // Test 2: Invalid token
    int r2 = csrf_validate_token(&svc, s1, "invalid");

    // Test 3: Expired token
    CsrfService svcShort;
    csrf_init(&svcShort, 10);
    const char *s2 = "sessB";
    char *t2 = csrf_issue_token(&svcShort, s2);
#if defined(_WIN32)
    Sleep(30);
#else
    struct timespec ts = {0, 30 * 1000000};
    nanosleep(&ts, NULL);
#endif
    int r3 = csrf_validate_token(&svcShort, s2, t2);

    // Test 4: Token with other session
    const char *s3 = "sessC";
    char *t3 = csrf_issue_token(&svc, s3);
    int r4 = csrf_validate_token(&svc, "other", t3);

    // Test 5: Replay attempt after rotation
    const char *s4 = "sessD";
    char *t4 = csrf_issue_token(&svc, s4);
    int first = csrf_validate_token(&svc, s4, t4);
    int second = csrf_validate_token(&svc, s4, t4);
    int r5 = first && !second;

    printf("%s\n", r1 ? "true" : "false");
    printf("%s\n", r2 ? "true" : "false");
    printf("%s\n", r3 ? "true" : "false");
    printf("%s\n", r4 ? "true" : "false");
    printf("%s\n", r5 ? "true" : "false");

    free(t1);
    free(t2);
    free(t3);
    free(t4);
    csrf_free(&svc);
    csrf_free(&svcShort);
    return 0;
}