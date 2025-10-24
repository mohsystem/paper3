#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

// Minimal SHA-256 implementation in C
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    unsigned char data[64];
    size_t datalen;
} sha256_ctx;

static uint32_t rotr32(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static uint32_t ch32(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static uint32_t maj32(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t ep0(uint32_t x){ return rotr32(x,2) ^ rotr32(x,13) ^ rotr32(x,22); }
static uint32_t ep1(uint32_t x){ return rotr32(x,6) ^ rotr32(x,11) ^ rotr32(x,25); }
static uint32_t sig0(uint32_t x){ return rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3); }
static uint32_t sig1(uint32_t x){ return rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10); }

static const uint32_t K256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static void sha256_init(sha256_ctx* ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul;
    ctx->state[1] = 0xbb67ae85ul;
    ctx->state[2] = 0x3c6ef372ul;
    ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful;
    ctx->state[5] = 0x9b05688cul;
    ctx->state[6] = 0x1f83d9abul;
    ctx->state[7] = 0x5be0cd19ul;
}

static void sha256_transform(sha256_ctx* ctx, const unsigned char data[]){
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)data[i*4] << 24) | ((uint32_t)data[i*4+1] << 16) | ((uint32_t)data[i*4+2] << 8) | ((uint32_t)data[i*4+3]);
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + ep1(e) + ch32(e,f,g) + K256[i] + m[i];
        uint32_t t2 = ep0(a) + maj32(a,b,c);
        h = g; g = f; f = e;
        e = d + t1;
        d = c; c = b; b = a;
        a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_update(sha256_ctx* ctx, const unsigned char* data, size_t len){
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(sha256_ctx* ctx, unsigned char hash[32]){
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    for (int j = 7; j >= 0; --j) {
        ctx->data[56 + (7 - j)] = (unsigned char)((ctx->bitlen >> (j * 8)) & 0xFFu);
    }
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xFFu;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xFFu;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xFFu;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xFFu;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xFFu;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xFFu;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xFFu;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xFFu;
    }
}

static void sha256(const unsigned char* data, size_t len, unsigned char out[32]){
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

// Helpers
static int secure_random_bytes(unsigned char* out, size_t len){
    if (out == NULL || len == 0) return 0;
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t rd = fread(out, 1, len, f);
        fclose(f);
        if (rd == len) return 1;
    }
    return 0; // fail closed if secure source not available
}

static void to_hex(const unsigned char* in, size_t len, char* out, size_t outsz){
    static const char* hex = "0123456789abcdef";
    if (outsz < len*2 + 1) {
        if (outsz > 0) out[0] = '\0';
        return;
    }
    for (size_t i = 0; i < len; ++i) {
        out[i*2] = hex[in[i] >> 4];
        out[i*2+1] = hex[in[i] & 0x0F];
    }
    out[len*2] = '\0';
}

static int consttime_eq(const unsigned char* a, const unsigned char* b, size_t len){
    unsigned char r = 0;
    for (size_t i = 0; i < len; ++i) {
        r |= (unsigned char)(a[i] ^ b[i]);
    }
    return r == 0;
}

static int consttime_eq_str(const char* a, const char* b){
    size_t la = strlen(a), lb = strlen(b);
    size_t l = la > lb ? la : lb;
    unsigned char r = 0;
    for (size_t i = 0; i < l; ++i) {
        unsigned char ca = i < la ? (unsigned char)a[i] : 0;
        unsigned char cb = i < lb ? (unsigned char)b[i] : 0;
        r |= (unsigned char)(ca ^ cb);
    }
    return (la == lb) && (r == 0);
}

// Password policy
static int is_strong_password(const char* pw){
    if (pw == NULL) return 0;
    size_t n = strlen(pw);
    if (n < 12 || n > 128) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)pw[i];
        if (c == ' ') return 0;
        if (c >= 'A' && c <= 'Z') hasU = 1;
        else if (c >= 'a' && c <= 'z') hasL = 1;
        else if (c >= '0' && c <= '9') hasD = 1;
        else hasS = 1;
    }
    const char* common[] = {
        "password","12345678","qwertyui","letmein123","adminadmin",
        "password1","iloveyou","welcome123","monkey123","abc123abc"
    };
    char lower[256];
    if (n >= sizeof(lower)) return 0;
    for (size_t i = 0; i < n; ++i) lower[i] = (char)tolower((unsigned char)pw[i]);
    lower[n] = '\0';
    for (size_t i = 0; i < sizeof(common)/sizeof(common[0]); ++i) {
        if (strstr(lower, common[i]) != NULL) return 0;
    }
    return hasU && hasL && hasD && hasS;
}

static int is_valid_username(const char* u){
    if (u == NULL) return 0;
    size_t n = strlen(u);
    if (n < 3 || n > 32) return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 1;
}

// Data structures
#define MAX_USERS 200
#define MAX_SESSIONS 400

typedef struct {
    char username[33];
    unsigned char salt[16];
    unsigned char hash[32];
    time_t pw_set;
    int in_use;
} user_t;

typedef struct {
    user_t users[MAX_USERS];
} user_store_t;

typedef struct {
    char token[65]; // hex of 32 bytes
    char username[33];
    time_t expiry;
    int in_use;
} session_t;

typedef struct {
    session_t sessions[MAX_SESSIONS];
    int ttl_seconds;
} session_store_t;

static void user_store_init(user_store_t* s){
    if (!s) return;
    memset(s, 0, sizeof(*s));
}
static void session_store_init(session_store_t* s){
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->ttl_seconds = 900;
}

static int user_store_find(user_store_t* s, const char* username){
    if (!s || !username) return -1;
    for (int i = 0; i < MAX_USERS; ++i) {
        if (s->users[i].in_use && strcmp(s->users[i].username, username) == 0) return i;
    }
    return -1;
}

static int user_store_add(user_store_t* s, const char* username, const char* password){
    if (!s || !username || !password) return 0;
    if (!is_valid_username(username) || !is_strong_password(password)) return 0;
    if (user_store_find(s, username) != -1) return 0;
    int slot = -1;
    for (int i = 0; i < MAX_USERS; ++i) { if (!s->users[i].in_use) { slot = i; break; } }
    if (slot < 0) return 0;

    unsigned char salt[16];
    if (!secure_random_bytes(salt, sizeof(salt))) return 0;

    size_t pwlen = strlen(password);
    unsigned char* buf = (unsigned char*)malloc(16 + pwlen);
    if (!buf) return 0;
    memcpy(buf, salt, 16);
    memcpy(buf + 16, password, pwlen);

    unsigned char digest[32];
    sha256(buf, 16 + pwlen, digest);
    // Clear sensitive temp buffer
    memset(buf, 0, 16 + pwlen);
    free(buf);

    user_t* u = &s->users[slot];
    memset(u, 0, sizeof(*u));
    snprintf(u->username, sizeof(u->username), "%s", username);
    memcpy(u->salt, salt, 16);
    memcpy(u->hash, digest, 32);
    u->pw_set = time(NULL);
    u->in_use = 1;
    return 1;
}

static int user_store_verify(user_store_t* s, const char* username, const char* password){
    if (!s || !username || !password) return 0;
    int idx = user_store_find(s, username);
    if (idx < 0) return 0;
    user_t* u = &s->users[idx];
    // password expiration 90 days
    time_t now = time(NULL);
    if (now - u->pw_set > 90 * 24 * 3600) return 0;

    size_t pwlen = strlen(password);
    unsigned char* buf = (unsigned char*)malloc(16 + pwlen);
    if (!buf) return 0;
    memcpy(buf, u->salt, 16);
    memcpy(buf + 16, password, pwlen);
    unsigned char digest[32];
    sha256(buf, 16 + pwlen, digest);
    memset(buf, 0, 16 + pwlen);
    free(buf);

    return consttime_eq(digest, u->hash, 32);
}

static void session_store_set_ttl(session_store_t* ss, int seconds){
    if (!ss) return;
    if (seconds > 0 && seconds <= 24*3600) ss->ttl_seconds = seconds;
}

static void session_store_cleanup(session_store_t* ss){
    if (!ss) return;
    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (ss->sessions[i].in_use && now > ss->sessions[i].expiry) {
            memset(&ss->sessions[i], 0, sizeof(ss->sessions[i]));
        }
    }
}

static int session_store_create(session_store_t* ss, const char* username, char* out_token, size_t out_tokensz){
    if (!ss || !username || !out_token) return 0;
    unsigned char rnd[32];
    if (!secure_random_bytes(rnd, sizeof(rnd))) return 0;
    char hex[65];
    to_hex(rnd, sizeof(rnd), hex, sizeof(hex));
    if (out_tokensz < sizeof(hex)) return 0;

    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; ++i) { if (!ss->sessions[i].in_use) { slot = i; break; } }
    if (slot < 0) return 0;

    session_t* s = &ss->sessions[slot];
    memset(s, 0, sizeof(*s));
    snprintf(s->token, sizeof(s->token), "%s", hex);
    snprintf(s->username, sizeof(s->username), "%s", username);
    s->expiry = time(NULL) + ss->ttl_seconds;
    s->in_use = 1;

    snprintf(out_token, out_tokensz, "%s", s->token);
    return 1;
}

static int session_store_validate(session_store_t* ss, const char* token, char* out_username, size_t out_usernamesz){
    if (!ss || !token) return 0;
    session_store_cleanup(ss);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (ss->sessions[i].in_use) {
            if (consttime_eq_str(ss->sessions[i].token, token)) {
                if (time(NULL) > ss->sessions[i].expiry) {
                    memset(&ss->sessions[i], 0, sizeof(ss->sessions[i]));
                    return 0;
                }
                if (out_username && out_usernamesz > 0) {
                    snprintf(out_username, out_usernamesz, "%s", ss->sessions[i].username);
                }
                return 1;
            }
        }
    }
    return 0;
}

static int session_store_logout(session_store_t* ss, const char* token){
    if (!ss || !token) return 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (ss->sessions[i].in_use && consttime_eq_str(ss->sessions[i].token, token)) {
            memset(&ss->sessions[i], 0, sizeof(ss->sessions[i]));
            return 1;
        }
    }
    return 0;
}

// Facade
typedef struct {
    user_store_t users;
    session_store_t sessions;
} auth_service_t;

static void auth_service_init(auth_service_t* a){
    if (!a) return;
    user_store_init(&a->users);
    session_store_init(&a->sessions);
}
static int auth_register(auth_service_t* a, const char* username, const char* password){
    if (!a) return 0;
    return user_store_add(&a->users, username, password);
}
static int auth_login(auth_service_t* a, const char* username, const char* password, char* out_token, size_t out_tokensz){
    if (!a) return 0;
    if (!user_store_verify(&a->users, username, password)) return 0;
    return session_store_create(&a->sessions, username, out_token, out_tokensz);
}
static int auth_validate(auth_service_t* a, const char* token, char* out_username, size_t out_usernamesz){
    if (!a) return 0;
    return session_store_validate(&a->sessions, token, out_username, out_usernamesz);
}
static int auth_logout(auth_service_t* a, const char* token){
    if (!a) return 0;
    return session_store_logout(&a->sessions, token);
}
static void auth_set_ttl(auth_service_t* a, int seconds){
    if (!a) return;
    session_store_set_ttl(&a->sessions, seconds);
}

// Tests
static void print_result(const char* name, int ok){
    printf("%s: %s\n", name, ok ? "PASS" : "FAIL");
}

int main(void){
    auth_service_t svc;
    auth_service_init(&svc);

    // Test 1: Register and login success
    int t1 = auth_register(&svc, "alice_user", "Str0ng!Passw0rd");
    char token1[65]; memset(token1, 0, sizeof(token1));
    int t1b = auth_login(&svc, "alice_user", "Str0ng!Passw0rd", token1, sizeof(token1));
    char uname1[33]; memset(uname1, 0, sizeof(uname1));
    int t1c = auth_validate(&svc, token1, uname1, sizeof(uname1));
    print_result("Test1_Register", t1);
    print_result("Test1_Login", t1b);
    print_result("Test1_Validate", t1c && strcmp(uname1, "alice_user") == 0);

    // Test 2: Duplicate registration
    int t2 = !auth_register(&svc, "alice_user", "An0ther!GoodOne");
    print_result("Test2_DuplicateRegister", t2);

    // Test 3: Weak password rejected
    int t3 = !auth_register(&svc, "bob", "weakpass");
    print_result("Test3_WeakPasswordRejected", t3);

    // Test 4: Wrong password login fails
    int t4a = auth_register(&svc, "charlie", "Anoth3r!StrongOne");
    char token4[65]; memset(token4, 0, sizeof(token4));
    int t4b = !auth_login(&svc, "charlie", "WrongPass!23", token4, sizeof(token4));
    print_result("Test4_RegisterCharlie", t4a);
    print_result("Test4_LoginWrongPassword", t4b);

    // Test 5: Session expiration
    int t5a = auth_register(&svc, "diana", "Sup3r!Secur3PW");
    auth_set_ttl(&svc, 1);
    char token5[65]; memset(token5, 0, sizeof(token5));
    int t5b = auth_login(&svc, "diana", "Sup3r!Secur3PW", token5, sizeof(token5));
    // Sleep ~1.2 seconds
#if defined(_WIN32)
    Sleep(1200);
#else
    struct timespec ts; ts.tv_sec = 1; ts.tv_nsec = 200000000L; nanosleep(&ts, NULL);
#endif
    char uname5[33]; memset(uname5, 0, sizeof(uname5));
    int t5c = !auth_validate(&svc, token5, uname5, sizeof(uname5));
    print_result("Test5_RegisterDiana", t5a);
    print_result("Test5_LoginDiana", t5b);
    print_result("Test5_SessionExpired", t5c);

    // Logout test (extra within 5th series)
    char token6[65]; memset(token6, 0, sizeof(token6));
    int t6a = auth_login(&svc, "alice_user", "Str0ng!Passw0rd", token6, sizeof(token6));
    int t6b = auth_logout(&svc, token6);
    char uname6[33]; memset(uname6, 0, sizeof(uname6));
    int t6c = !auth_validate(&svc, token6, uname6, sizeof(uname6));
    print_result("Test5b_Logout", t6a && t6b && t6c);

    // Clear sensitive buffers before exit
    memset(token1, 0, sizeof(token1));
    memset(token4, 0, sizeof(token4));
    memset(token5, 0, sizeof(token5));
    memset(token6, 0, sizeof(token6));
    return 0;
}