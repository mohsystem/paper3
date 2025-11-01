#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Minimal SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256 and in-memory DB with safe patterns.

static inline uint32_t rotr32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t BSIG0(uint32_t x) { return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22); }
static inline uint32_t BSIG1(uint32_t x) { return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25); }
static inline uint32_t SSIG0(uint32_t x) { return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3); }
static inline uint32_t SSIG1(uint32_t x) { return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10); }

static const uint32_t Kc[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

typedef struct {
    uint64_t bitlen;
    uint32_t state[8];
    uint8_t buffer[64];
    size_t buffer_len;
} sha256_ctx;

static void sha256_init(sha256_ctx *ctx) {
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667u; ctx->state[1]=0xbb67ae85u; ctx->state[2]=0x3c6ef372u; ctx->state[3]=0xa54ff53au;
    ctx->state[4]=0x510e527fu; ctx->state[5]=0x9b05688cu; ctx->state[6]=0x1f83d9abu; ctx->state[7]=0x5be0cd19u;
    ctx->buffer_len = 0;
}

static void sha256_transform(sha256_ctx *ctx, const uint8_t data[64]) {
    uint32_t w[64];
    for (int i=0;i<16;i++) {
        w[i] = ((uint32_t)data[i*4+0]<<24) | ((uint32_t)data[i*4+1]<<16) | ((uint32_t)data[i*4+2]<<8) | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++) w[i] = SSIG1(w[i-2]) + w[i-7] + SSIG0(w[i-15]) + w[i-16];
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3],e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for (int i=0;i<64;i++) {
        uint32_t t1 = h + BSIG1(e) + Ch(e,f,g) + Kc[i] + w[i];
        uint32_t t2 = BSIG0(a) + Maj(a,b,c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    while (len > 0) {
        size_t to_copy = 64 - ctx->buffer_len;
        if (to_copy > len) to_copy = len;
        memcpy(ctx->buffer + ctx->buffer_len, data, to_copy);
        ctx->buffer_len += to_copy;
        data += to_copy;
        len -= to_copy;
        if (ctx->buffer_len == 64) {
            sha256_transform(ctx, ctx->buffer);
            ctx->bitlen += 512;
            ctx->buffer_len = 0;
        }
    }
}

static void sha256_final(sha256_ctx *ctx, uint8_t out[32]) {
    ctx->bitlen += ctx->buffer_len * 8;
    ctx->buffer[ctx->buffer_len++] = 0x80;
    if (ctx->buffer_len > 56) {
        while (ctx->buffer_len < 64) ctx->buffer[ctx->buffer_len++] = 0x00;
        sha256_transform(ctx, ctx->buffer);
        ctx->buffer_len = 0;
    }
    while (ctx->buffer_len < 56) ctx->buffer[ctx->buffer_len++] = 0x00;
    for (int i=7;i>=0;i--) {
        ctx->buffer[ctx->buffer_len++] = (uint8_t)((ctx->bitlen >> (i*8)) & 0xFFu);
    }
    sha256_transform(ctx, ctx->buffer);
    for (int i=0;i<8;i++) {
        out[i*4+0] = (uint8_t)((ctx->state[i] >> 24) & 0xFFu);
        out[i*4+1] = (uint8_t)((ctx->state[i] >> 16) & 0xFFu);
        out[i*4+2] = (uint8_t)((ctx->state[i] >> 8) & 0xFFu);
        out[i*4+3] = (uint8_t)(ctx->state[i] & 0xFFu);
    }
}

// HMAC-SHA256
static void hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t out[32]) {
    const size_t blockSize = 64;
    uint8_t k[64]; memset(k, 0, sizeof(k));
    if (key_len > blockSize) {
        sha256_ctx kctx; sha256_init(&kctx);
        sha256_update(&kctx, key, key_len);
        sha256_final(&kctx, k);
    } else {
        memcpy(k, key, key_len);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i=0;i<64;i++) { ipad[i] = (uint8_t)(k[i] ^ 0x36u); opad[i] = (uint8_t)(k[i] ^ 0x5cu); }
    sha256_ctx ictx; sha256_init(&ictx);
    sha256_update(&ictx, ipad, 64);
    sha256_update(&ictx, data, data_len);
    uint8_t inner[32]; sha256_final(&ictx, inner);

    sha256_ctx octx; sha256_init(&octx);
    sha256_update(&octx, opad, 64);
    sha256_update(&octx, inner, 32);
    sha256_final(&octx, out);

    volatile uint8_t wipe = 0;
    for (size_t i=0;i<64;i++) { wipe ^= k[i]; k[i]=0; }
    for (size_t i=0;i<64;i++) { wipe ^= ipad[i]; ipad[i]=0; }
    for (size_t i=0;i<64;i++) { wipe ^= opad[i]; opad[i]=0; }
    for (size_t i=0;i<32;i++) { wipe ^= inner[i]; }
    (void)wipe;
    memset((void*)inner, 0, sizeof(inner));
}

// PBKDF2-HMAC-SHA256
static int pbkdf2_hmac_sha256(const uint8_t *password, size_t pw_len, const uint8_t *salt, size_t salt_len, uint32_t iterations, uint8_t *out, size_t dkLen) {
    if (iterations == 0 || dkLen == 0) return 0;
    uint32_t blockCount = (uint32_t)((dkLen + 31) / 32);
    size_t pos = 0;
    for (uint32_t i=1; i<=blockCount; i++) {
        // U1 = HMAC(P, S || INT(i))
        uint8_t be[4] = { (uint8_t)(i>>24), (uint8_t)(i>>16), (uint8_t)(i>>8), (uint8_t)(i) };
        size_t msg_len = salt_len + 4;
        uint8_t *msg = (uint8_t*)malloc(msg_len);
        if (!msg) return 0;
        memcpy(msg, salt, salt_len);
        memcpy(msg + salt_len, be, 4);

        uint8_t u[32], t[32];
        hmac_sha256(password, pw_len, msg, msg_len, u);
        memcpy(t, u, 32);
        for (uint32_t j=2; j<=iterations; j++) {
            hmac_sha256(password, pw_len, u, 32, u);
            for (size_t k=0;k<32;k++) t[k] ^= u[k];
        }
        size_t to_copy = (dkLen - pos) < 32 ? (dkLen - pos) : 32;
        memcpy(out + pos, t, to_copy);
        pos += to_copy;

        volatile uint8_t wipe = 0;
        for (size_t k=0;k<msg_len;k++) { wipe ^= msg[k]; }
        free(msg);
        for (size_t k=0;k<32;k++) { wipe ^= u[k]; wipe ^= t[k]; }
        (void)wipe;
        memset(u, 0, sizeof(u));
        memset(t, 0, sizeof(t));
    }
    return 1;
}

static int constant_time_equal(const uint8_t *a, const uint8_t *b, size_t len) {
    uint8_t r = 0;
    for (size_t i=0;i<len;i++) r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}

static int get_random_bytes(uint8_t *out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t total = 0;
    while (total < len) {
        size_t n = fread(out + total, 1, len - total, f);
        if (n == 0) { fclose(f); return 0; }
        total += n;
    }
    fclose(f);
    return 1;
}

// In-memory DB
#define MAX_USERS 16
typedef struct {
    char username[33]; // 32 + null
    uint8_t salt[16];
    uint8_t hash[32];
    uint32_t iterations;
    int used;
} UserRecord;

typedef struct {
    UserRecord records[MAX_USERS];
} SecureAuthDB;

static void init_db(SecureAuthDB *db) {
    for (int i=0;i<MAX_USERS;i++) db->records[i].used = 0;
}

static int validate_username(const char *u) {
    size_t n = u ? strlen(u) : 0;
    if (n < 3 || n > 32) return 0;
    for (size_t i=0;i<n;i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

static int password_policy(const char *p) {
    size_t n = p ? strlen(p) : 0;
    if (n < 10 || n > 128) return 0;
    int up=0, lo=0, di=0, sp=0;
    for (size_t i=0;i<n;i++) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) up=1;
        else if (islower(c)) lo=1;
        else if (isdigit(c)) di=1;
        else sp=1;
    }
    return up && lo && di && sp;
}

static UserRecord* find_user(SecureAuthDB *db, const char *username) {
    for (int i=0;i<MAX_USERS;i++) {
        if (db->records[i].used && strncmp(db->records[i].username, username, 32) == 0) {
            return &db->records[i];
        }
    }
    return NULL;
}

static int register_user(SecureAuthDB *db, const char *username, const char *password) {
    if (!validate_username(username)) return 0;
    if (!password_policy(password)) return 0;
    if (find_user(db, username)) return 0;
    int slot = -1;
    for (int i=0;i<MAX_USERS;i++) {
        if (!db->records[i].used) { slot = i; break; }
    }
    if (slot < 0) return 0;

    uint8_t salt[16];
    if (!get_random_bytes(salt, sizeof(salt))) return 0;

    uint8_t hash[32];
    if (!pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, sizeof(salt), 210000, hash, sizeof(hash))) {
        return 0;
    }

    UserRecord *rec = &db->records[slot];
    memset(rec, 0, sizeof(*rec));
    strncpy(rec->username, username, 32);
    memcpy(rec->salt, salt, sizeof(salt));
    memcpy(rec->hash, hash, sizeof(hash));
    rec->iterations = 210000;
    rec->used = 1;

    volatile uint8_t wipe = 0;
    for (size_t i=0;i<sizeof(salt);i++) { wipe ^= salt[i]; }
    for (size_t i=0;i<sizeof(hash);i++) { wipe ^= hash[i]; }
    (void)wipe;
    memset(salt, 0, sizeof(salt));
    memset(hash, 0, sizeof(hash));
    return 1;
}

static int authenticate(SecureAuthDB *db, const char *username, const char *password) {
    if (!validate_username(username)) return 0;
    UserRecord *rec = find_user(db, username);
    if (!rec) return 0;
    uint8_t computed[32];
    if (!pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), rec->salt, sizeof(rec->salt), rec->iterations, computed, sizeof(computed))) {
        return 0;
    }
    int ok = constant_time_equal(rec->hash, computed, sizeof(computed));
    volatile uint8_t wipe = 0;
    for (size_t i=0;i<sizeof(computed);i++) { wipe ^= computed[i]; }
    (void)wipe;
    memset(computed, 0, sizeof(computed));
    return ok;
}

static const char* login(SecureAuthDB *db, const char *username, const char *password) {
    return authenticate(db, username, password) ? "Redirect: /dashboard" : "Redirect: /login";
}

int main(void) {
    SecureAuthDB db;
    init_db(&db);

    printf("Register alice: %s\n", register_user(&db, "alice", "Str0ng!Pass1") ? "true" : "false");
    printf("Register bob: %s\n", register_user(&db, "bob", "B0b$Secure#2024") ? "true" : "false");
    printf("Register charlie: %s\n", register_user(&db, "charlie", "Ch@rlie2025!!") ? "true" : "false");

    const char* tests[5][2] = {
        {"alice", "Str0ng!Pass1"},
        {"alice", "wrongpassword!"},
        {"unknown", "AnyPassw0rd!"},
        {"bad user", "Xx1!invalid"},
        {"bob", "B0b$Secure#2024"}
    };

    for (int i=0;i<5;i++) {
        const char* u = tests[i][0];
        const char* p = tests[i][1];
        const char* res = login(&db, u, p);
        printf("Test %d login(%s, [redacted]): %s\n", i+1, u, res);
    }
    return 0;
}