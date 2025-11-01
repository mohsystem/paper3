#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <errno.h>
#endif

// Secure login with PBKDF2-HMAC-SHA256, per-user random salts, and input validation.

// ---------- SHA-256 Implementation ----------
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static inline uint32_t rotr32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x) { return rotr32(x,2) ^ rotr32(x,13) ^ rotr32(x,22); }
static inline uint32_t ep1(uint32_t x) { return rotr32(x,6) ^ rotr32(x,11) ^ rotr32(x,25); }
static inline uint32_t sig0(uint32_t x) { return rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x) { return rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10); }

static const uint32_t K256[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_init(SHA256_CTX* ctx) {
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
    ctx->bitlen = 0; ctx->datalen = 0;
    memset(ctx->data, 0, sizeof(ctx->data));
}

static inline uint32_t be32(const uint8_t b[4]) {
    return ((uint32_t)b[0]<<24) | ((uint32_t)b[1]<<16) | ((uint32_t)b[2]<<8) | (uint32_t)b[3];
}

static void sha256_transform(SHA256_CTX* ctx, const uint8_t block[64]) {
    uint32_t m[64];
    for (int i=0;i<16;i++) m[i] = be32(block + i*4);
    for (int i=16;i<64;i++) m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];

    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i=0;i<64;i++) {
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + K256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* in, size_t len) {
    for (size_t i=0;i<len;i++) {
        ctx->data[ctx->datalen++] = in[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t out[32]) {
    uint64_t bits = ctx->bitlen + ctx->datalen*8;
    ctx->data[ctx->datalen++] = 0x80;
    if (ctx->datalen > 56) {
        while (ctx->datalen < 64) ctx->data[ctx->datalen++] = 0;
        sha256_transform(ctx, ctx->data);
        ctx->datalen = 0;
    }
    while (ctx->datalen < 56) ctx->data[ctx->datalen++] = 0;
    uint8_t lenbe[8];
    for (int i=7;i>=0;i--) { lenbe[i] = (uint8_t)(bits & 0xFF); bits >>= 8; }
    for (int i=0;i<8;i++) ctx->data[56+i] = lenbe[i];
    sha256_transform(ctx, ctx->data);
    for (int i=0;i<8;i++) {
        out[i*4+0] = (uint8_t)((ctx->state[i] >> 24) & 0xFF);
        out[i*4+1] = (uint8_t)((ctx->state[i] >> 16) & 0xFF);
        out[i*4+2] = (uint8_t)((ctx->state[i] >> 8) & 0xFF);
        out[i*4+3] = (uint8_t)(ctx->state[i] & 0xFF);
    }
    memset(ctx, 0, sizeof(*ctx));
}

// ---------- HMAC-SHA256 ----------
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* msg, size_t msglen, uint8_t out[32]) {
    uint8_t kopad[64], kipad[64], khash[32];
    if (keylen > 64) {
        SHA256_CTX c; sha256_init(&c); sha256_update(&c, key, keylen); sha256_final(&c, khash);
        key = khash; keylen = 32;
    }
    memset(kopad, 0, sizeof(kopad));
    memset(kipad, 0, sizeof(kipad));
    memcpy(kopad, key, keylen);
    memcpy(kipad, key, keylen);
    for (int i=0;i<64;i++) { kipad[i]^=0x36; kopad[i]^=0x5c; }
    uint8_t inner[32];
    SHA256_CTX ictx; sha256_init(&ictx); sha256_update(&ictx, kipad, 64); sha256_update(&ictx, msg, msglen); sha256_final(&ictx, inner);
    SHA256_CTX octx; sha256_init(&octx); sha256_update(&octx, kopad, 64); sha256_update(&octx, inner, 32); sha256_final(&octx, out);
    memset(kopad,0,sizeof(kopad)); memset(kipad,0,sizeof(kipad)); memset(khash,0,sizeof(khash)); memset(inner,0,sizeof(inner));
}

// ---------- PBKDF2-HMAC-SHA256 ----------
static int pbkdf2_hmac_sha256(const uint8_t* password, size_t pwlen,
                              const uint8_t* salt, size_t saltlen,
                              uint32_t iterations,
                              uint8_t* out, size_t dkLen) {
    if (iterations < 1 || dkLen == 0) return 0;
    const size_t hLen = 32;
    uint32_t blocks = (uint32_t)((dkLen + hLen - 1) / hLen);
    uint8_t* U = (uint8_t*)calloc(hLen, 1);
    uint8_t* T = (uint8_t*)calloc(hLen, 1);
    uint8_t* saltBlock = (uint8_t*)malloc(saltlen + 4);
    if (!U || !T || !saltBlock) { free(U); free(T); free(saltBlock); return 0; }
    memcpy(saltBlock, salt, saltlen);
    for (uint32_t i=1; i<=blocks; i++) {
        saltBlock[saltlen+0] = (uint8_t)((i >> 24) & 0xFF);
        saltBlock[saltlen+1] = (uint8_t)((i >> 16) & 0xFF);
        saltBlock[saltlen+2] = (uint8_t)((i >> 8) & 0xFF);
        saltBlock[saltlen+3] = (uint8_t)(i & 0xFF);
        hmac_sha256(password, pwlen, saltBlock, saltlen+4, U);
        memcpy(T, U, hLen);
        for (uint32_t j=2; j<=iterations; j++) {
            hmac_sha256(password, pwlen, U, hLen, U);
            for (size_t k=0;k<hLen;k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i - 1) * hLen;
        size_t cp = hLen;
        if (offset + cp > dkLen) cp = dkLen - offset;
        memcpy(out + offset, T, cp);
    }
    // wipe and free
    memset(U,0,hLen); memset(T,0,hLen); memset(saltBlock,0,saltlen+4);
    free(U); free(T); free(saltBlock);
    return 1;
}

// ---------- CSPRNG ----------
static int get_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 1 : 0;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t off = 0;
    while (off < len) {
        size_t r = fread(buf + off, 1, len - off, f);
        if (r == 0) { fclose(f); return 0; }
        off += r;
    }
    fclose(f);
    return 1;
#endif
}

static int constant_time_eq(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t diff = 0;
    for (size_t i=0;i<n;i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

#define SALT_LEN 16
#define HASH_LEN 32
#define MAX_USERS 16

typedef struct {
    char username[33]; // up to 32 chars + NUL
    uint8_t salt[SALT_LEN];
    uint8_t hash[HASH_LEN];
    uint32_t iterations;
    int used;
} User;

typedef struct {
    User users[MAX_USERS];
    size_t count;
} InMemoryDB;

static int validateUsername(const char* username) {
    if (!username) return 0;
    size_t len = strlen(username);
    if (len < 3 || len > 32) return 0;
    for (size_t i=0;i<len;i++) {
        unsigned char c = (unsigned char)username[i];
        if (!(isalnum(c) || c=='.' || c=='_' || c=='-')) return 0;
    }
    return 1;
}

static int validatePassword(const char* password) {
    if (!password) return 0;
    size_t len = strlen(password);
    return (len >= 8 && len <= 128) ? 1 : 0;
}

static void initDB(InMemoryDB* db) {
    db->count = 0;
    for (size_t i=0;i<MAX_USERS;i++) db->users[i].used = 0;
}

static int addUser(InMemoryDB* db, const char* username, const char* password) {
    if (!validateUsername(username) || !validatePassword(password)) return 0;
    if (db->count >= MAX_USERS) return 0;
    for (size_t i=0;i<db->count;i++) {
        if (db->users[i].used && strcmp(db->users[i].username, username)==0) return 0;
    }
    User u;
    memset(&u, 0, sizeof(u));
    strncpy(u.username, username, 32);
    u.username[32] = '\0';
    if (!get_random_bytes(u.salt, SALT_LEN)) return 0;
    u.iterations = 210000;
    if (!pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), u.salt, SALT_LEN, u.iterations, u.hash, HASH_LEN)) return 0;
    u.used = 1;
    db->users[db->count++] = u;
    return 1;
}

static const User* findByUsername(const InMemoryDB* db, const char* username) {
    for (size_t i=0;i<db->count;i++) {
        if (db->users[i].used && strcmp(db->users[i].username, username)==0) return &db->users[i];
    }
    return NULL;
}

const char* handleLogin(InMemoryDB* db, const char* username, const char* password) {
    if (!validateUsername(username) || !validatePassword(password)) {
        return "REDIRECT:/login";
    }
    const User* u = findByUsername(db, username);
    if (!u) return "REDIRECT:/login";
    uint8_t computed[HASH_LEN];
    if (!pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), u->salt, SALT_LEN, u->iterations, computed, HASH_LEN)) {
        return "REDIRECT:/login";
    }
    int ok = constant_time_eq(computed, u->hash, HASH_LEN);
    memset(computed, 0, sizeof(computed));
    return ok ? "REDIRECT:/dashboard" : "REDIRECT:/login";
}

int main(void) {
    InMemoryDB db;
    initDB(&db);
    if (!addUser(&db, "alice", "CorrectHorse1!")) { printf("Init failed\n"); return 1; }
    if (!addUser(&db, "bob", "S3cureP@ssw0rd")) { printf("Init failed\n"); return 1; }

    const char* res1 = handleLogin(&db, "alice", "CorrectHorse1!");
    const char* res2 = handleLogin(&db, "alice", "wrongpass");
    const char* res3 = handleLogin(&db, "charlie", "NoSuchUser1!");
    const char* res4 = handleLogin(&db, "x", "SomeValid1!");
    const char* res5 = handleLogin(&db, "bob", "short");

    printf("Test 1: %s\n", res1);
    printf("Test 2: %s\n", res2);
    printf("Test 3: %s\n", res3);
    printf("Test 4: %s\n", res4);
    printf("Test 5: %s\n", res5);

    return 0;
}