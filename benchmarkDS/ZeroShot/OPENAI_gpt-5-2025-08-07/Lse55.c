#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

/*
 Minimal SHA-256 implementation (public domain) for single-file use.
*/
typedef struct {
    uint64_t bitlen;
    uint32_t state[8];
    uint8_t data[64];
    uint32_t datalen;
} SHA256_CTX;

static inline uint32_t ROTRIGHT32(uint32_t a, uint32_t b) { return ((a >> b) | (a << (32 - b))); }
static inline uint32_t CH32(uint32_t x, uint32_t y, uint32_t z) { return ((x & y) ^ (~x & z)); }
static inline uint32_t MAJ32(uint32_t x, uint32_t y, uint32_t z) { return ((x & y) ^ (x & z) ^ (y & z)); }
static inline uint32_t EP0_32(uint32_t x) { return ROTRIGHT32(x,2) ^ ROTRIGHT32(x,13) ^ ROTRIGHT32(x,22); }
static inline uint32_t EP1_32(uint32_t x) { return ROTRIGHT32(x,6) ^ ROTRIGHT32(x,11) ^ ROTRIGHT32(x,25); }
static inline uint32_t SIG0_32(uint32_t x) { return ROTRIGHT32(x,7) ^ ROTRIGHT32(x,18) ^ ((x) >> 3); }
static inline uint32_t SIG1_32(uint32_t x) { return ROTRIGHT32(x,17) ^ ROTRIGHT32(x,19) ^ ((x) >> 10); }

static const uint32_t k256_c[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform_c(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (uint32_t(data[j]) << 24) | (uint32_t(data[j + 1]) << 16) | (uint32_t(data[j + 2]) << 8) | (uint32_t(data[j + 3]));
    for ( ; i < 64; ++i)
        m[i] = SIG1_32(m[i - 2]) + m[i - 7] + SIG0_32(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1_32(e) + CH32(e,f,g) + k256_c[i] + m[i];
        t2 = EP0_32(a) + MAJ32(a,b,c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init_c(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

static void sha256_update_c(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform_c(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final_c(SHA256_CTX *ctx, uint8_t hash[]) {
    size_t i = ctx->datalen;

    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform_c(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = (uint8_t)ctx->bitlen;
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform_c(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xff;
    }
}

static void sha256_str(const char *data, size_t len, char out_hex[65]) {
    SHA256_CTX ctx;
    sha256_init_c(&ctx);
    sha256_update_c(&ctx, (const uint8_t*)data, len);
    uint8_t hash[32];
    sha256_final_c(&ctx, hash);
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out_hex[2*i] = hex[(hash[i] >> 4) & 0xF];
        out_hex[2*i+1] = hex[hash[i] & 0xF];
    }
    out_hex[64] = '\0';
}

static void kdf_iter_sha256_c(const char *password, const char *salt, int iterations, char out_hex[65]) {
    size_t pwlen = strlen(password);
    size_t slen = strlen(salt);
    size_t buflen = pwlen + slen + 2 + 64; // enough
    char *buf = (char*)malloc(buflen);
    if (!buf) { out_hex[0]='\0'; return; }
    snprintf(buf, buflen, "%s:%s", salt, password);
    char tmp[65];
    for (int i = 0; i < iterations; ++i) {
        size_t curlen = strlen(buf);
        char *cat = (char*)malloc(curlen + pwlen + slen + 3);
        if (!cat) { free(buf); out_hex[0]='\0'; return; }
        snprintf(cat, curlen + pwlen + slen + 3, "%s:%s:%s", buf, password, salt);
        free(buf);
        sha256_str(cat, strlen(cat), tmp);
        buf = strdup(tmp);
        free(cat);
        if (!buf) { out_hex[0]='\0'; return; }
    }
    strncpy(out_hex, buf, 65);
    out_hex[64] = '\0';
    free(buf);
}

static int constant_time_equals_c(const char *a, const char *b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char r = 0;
    for (size_t i = 0; i < la; ++i) r |= (unsigned char)a[i] ^ (unsigned char)b[i];
    return r == 0;
}

static int valid_username_c(const char *u) {
    if (!u) return 0;
    size_t n = strlen(u);
    if (n == 0 || n > 32) return 0;
    for (size_t i = 0; i < n; ++i) {
        char c = u[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              (c == '_'))) return 0;
    }
    return 1;
}

static void random_bytes_secure(uint8_t *out, size_t len) {
#ifdef _WIN32
    HCRYPTPROV hProvider = 0;
    if (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        if (CryptGenRandom(hProvider, (DWORD)len, out)) {
            CryptReleaseContext(hProvider, 0);
            return;
        }
        CryptReleaseContext(hProvider, 0);
    }
    // Fallback
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&out);
    for (size_t i = 0; i < len; ++i) out[i] = rand() & 0xFF;
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t r = fread(out, 1, len, f);
        fclose(f);
        if (r == len) return;
    }
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&out);
    for (size_t i = 0; i < len; ++i) out[i] = rand() & 0xFF;
#endif
}

static char *random_token_urlsafe_c(size_t bytes) {
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    uint8_t *rb = (uint8_t*)malloc(bytes);
    if (!rb) return NULL;
    random_bytes_secure(rb, bytes);
    char *out = (char*)malloc(bytes * 2 + 1);
    if (!out) { free(rb); return NULL; }
    for (size_t i = 0; i < bytes; ++i) {
        out[2*i] = alphabet[rb[i] % (sizeof(alphabet)-1)];
        out[2*i+1] = alphabet[(rb[i] + 73) % (sizeof(alphabet)-1)];
    }
    out[bytes*2] = '\0';
    free(rb);
    return out;
}

typedef struct {
    int authenticated;
    char *username;
    char *role;
    char *csrfToken;
} Session;

typedef struct {
    char *role;
    char *salt;
    char *expectedHash;
} UserRecord;

typedef struct UserNode {
    char *username;
    UserRecord rec;
    struct UserNode *next;
} UserNode;

static UserNode *USERS = NULL;

static void add_user_c(const char *username, const char *plainPassword, const char *role, const char *saltText) {
    char h[65];
    kdf_iter_sha256_c(plainPassword, saltText, 100000, h);
    UserNode *node = (UserNode*)calloc(1, sizeof(UserNode));
    node->username = strdup(username);
    node->rec.role = strdup(role);
    node->rec.salt = strdup(saltText);
    node->rec.expectedHash = strdup(h);
    node->next = USERS;
    USERS = node;
}

static UserRecord* find_user_c(const char *username) {
    UserNode *cur = USERS;
    while (cur) {
        if (strcmp(cur->username, username) == 0) return &cur->rec;
        cur = cur->next;
    }
    return NULL;
}

static char* str_concat3(const char *a, const char *b, const char *c) {
    size_t la = a ? strlen(a) : 0;
    size_t lb = b ? strlen(b) : 0;
    size_t lc = c ? strlen(c) : 0;
    char *out = (char*)malloc(la + lb + lc + 1);
    if (!out) return NULL;
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    memcpy(out + la + lb, c, lc);
    out[la + lb + lc] = '\0';
    return out;
}

char* login(Session *session, const char *method) {
    if (!session) return strdup("400 Bad Request");
    if (strcmp(method, "GET") != 0) return strdup("405 Method Not Allowed");
    if (session->csrfToken) { free(session->csrfToken); session->csrfToken = NULL; }
    session->csrfToken = random_token_urlsafe_c(32);
    const char *prefix = "Login Page (GET): Provide username, password, and CSRF token. CSRF=";
    char *out = str_concat3(prefix, session->csrfToken ? session->csrfToken : "", "");
    return out ? out : strdup("500 Internal Server Error");
}

char* do_login(Session *session, const char *method, const char *username, const char *password, const char *csrfToken) {
    if (!session) return strdup("400 Bad Request");
    if (strcmp(method, "POST") != 0) return strdup("405 Method Not Allowed");
    if (!session->csrfToken || !csrfToken || !constant_time_equals_c(session->csrfToken, csrfToken)) {
        return strdup("403 Forbidden: Invalid CSRF token");
    }
    if (!username || !password) return strdup("400 Bad Request");
    if (!valid_username_c(username)) return strdup("400 Bad Request: Invalid username format");

    UserRecord *rec = find_user_c(username);
    if (!rec) return strdup("401 Unauthorized: Invalid credentials");

    char computed[65];
    kdf_iter_sha256_c(password, rec->salt, 100000, computed);
    if (!constant_time_equals_c(computed, rec->expectedHash)) {
        return strdup("401 Unauthorized: Invalid credentials");
    }

    session->authenticated = 1;
    if (session->username) free(session->username);
    if (session->role) free(session->role);
    session->username = strdup(username);
    session->role = strdup(rec->role);
    if (session->csrfToken) { free(session->csrfToken); }
    session->csrfToken = random_token_urlsafe_c(32);

    char buf[256];
    snprintf(buf, sizeof(buf), "200 OK: Login successful for %s", username);
    return strdup(buf);
}

char* index(Session *session, const char *method) {
    if (!session) return strdup("400 Bad Request");
    if (strcmp(method, "GET") != 0) return strdup("405 Method Not Allowed");
    if (session->authenticated && session->username) {
        const char *prefix = "Index (GET): Welcome, ";
        const char *suffix = "!";
        char *tmp = str_concat3(prefix, session->username, suffix);
        return tmp ? tmp : strdup("500 Internal Server Error");
    } else {
        return strdup("Index (GET): Welcome, guest. Please login.");
    }
}

char* user_page(Session *session, const char *method) {
    if (!session) return strdup("400 Bad Request");
    if (strcmp(method, "GET") != 0) return strdup("405 Method Not Allowed");
    if (!session->authenticated) return strdup("401 Unauthorized: Please login");
    if (!session->role || (strcmp(session->role, "user") != 0 && strcmp(session->role, "admin") != 0)) {
        return strdup("403 Forbidden");
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "User Page (GET): Hello %s (role=%s)", session->username ? session->username : "?", session->role);
    return strdup(buf);
}

char* admin_page(Session *session, const char *method) {
    if (!session) return strdup("400 Bad Request");
    if (strcmp(method, "GET") != 0) return strdup("405 Method Not Allowed");
    if (!session->authenticated) return strdup("401 Unauthorized: Please login");
    if (!session->role || strcmp(session->role, "admin") != 0) return strdup("403 Forbidden: Admins only");
    char buf[256];
    snprintf(buf, sizeof(buf), "Admin Page (GET): Welcome admin %s", session->username ? session->username : "?");
    return strdup(buf);
}

static void session_init(Session *s) {
    s->authenticated = 0;
    s->username = NULL;
    s->role = NULL;
    s->csrfToken = NULL;
}

static void session_free(Session *s) {
    if (!s) return;
    if (s->username) free(s->username);
    if (s->role) free(s->role);
    if (s->csrfToken) free(s->csrfToken);
    s->username = s->role = s->csrfToken = NULL;
}

int main(void) {
    add_user_c("alice", "password123!", "user", "NaClAlice");
    add_user_c("admin", "Admin#2025", "admin", "NaClAdmin");

    // Test case 1: GET login page
    Session s1; session_init(&s1);
    char *r = login(&s1, "GET"); printf("TC1-login: %s\n", r); free(r);

    // Test case 2: bad CSRF
    Session s2; session_init(&s2);
    r = login(&s2, "GET"); printf("TC2-login: %s\n", r); free(r);
    r = do_login(&s2, "POST", "alice", "password123!", "badtoken"); printf("TC2-do_login: %s\n", r); free(r);
    r = index(&s2, "GET"); printf("TC2-index: %s\n", r); free(r);

    // Test case 3: wrong password
    Session s3; session_init(&s3);
    r = login(&s3, "GET"); printf("TC3-login: %s\n", r); free(r);
    r = do_login(&s3, "POST", "alice", "wrongpass", s3.csrfToken); printf("TC3-do_login: %s\n", r); free(r);
    r = index(&s3, "GET"); printf("TC3-index: %s\n", r); free(r);

    // Test case 4: user login then admin page
    Session s4; session_init(&s4);
    r = login(&s4, "GET"); printf("TC4-login: %s\n", r); free(r);
    r = do_login(&s4, "POST", "alice", "password123!", s4.csrfToken); printf("TC4-do_login: %s\n", r); free(r);
    r = user_page(&s4, "GET"); printf("TC4-user_page: %s\n", r); free(r);
    r = admin_page(&s4, "GET"); printf("TC4-admin_page: %s\n", r); free(r);

    // Test case 5: admin login
    Session s5; session_init(&s5);
    r = login(&s5, "GET"); printf("TC5-login: %s\n", r); free(r);
    r = do_login(&s5, "POST", "admin", "Admin#2025", s5.csrfToken); printf("TC5-do_login: %s\n", r); free(r);
    r = index(&s5, "GET"); printf("TC5-index: %s\n", r); free(r);
    r = admin_page(&s5, "GET"); printf("TC5-admin_page: %s\n", r); free(r);

    session_free(&s1);
    session_free(&s2);
    session_free(&s3);
    session_free(&s4);
    session_free(&s5);
    return 0;
}