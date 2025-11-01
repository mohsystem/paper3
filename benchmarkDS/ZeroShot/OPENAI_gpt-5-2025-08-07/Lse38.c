#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Minimal SHA-256 implementation
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

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
static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i=0;i<16;i++) {
        m[i] = ((uint32_t)data[i*4] << 24) | ((uint32_t)data[i*4+1] << 16) | ((uint32_t)data[i*4+2] << 8) | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++) m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i=0;i<64;i++) {
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len){
    for (size_t i=0;i<len;i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[32]){
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
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i=0;i<4;i++) {
        for (int j=0;j<8;j++) hash[i + j*4] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xff);
    }
}

static void sha256_bytes(const uint8_t* data, size_t len, uint8_t out[32]) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

static int ct_equal(const uint8_t* a, const uint8_t* b, size_t n){
    uint8_t diff = 0;
    for (size_t i=0;i<n;i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

typedef struct {
    char username[64];
    uint8_t salt[16];
    uint8_t hash[32];
    int used;
} User;

#define MAX_USERS 16

typedef struct {
    User users[MAX_USERS];
} Auth;

static void random_bytes(uint8_t* buf, size_t n){
#if defined(_WIN32)
    // Fallback simple PRNG for demo; in real code use BCryptGenRandom
    srand((unsigned)time(NULL));
    for (size_t i=0;i<n;i++) buf[i] = (uint8_t)(rand() & 0xFF);
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(buf, 1, n, f);
        fclose(f);
    } else {
        srand((unsigned)time(NULL));
        for (size_t i=0;i<n;i++) buf[i] = (uint8_t)(rand() & 0xFF);
    }
#endif
}

static void derive_key(const char* password, const uint8_t* salt, size_t saltlen, int iterations, uint8_t out[32]){
    // Simple iterative hash: h = SHA256(salt || password); repeat with h||salt||password
    size_t passlen = strlen(password);
    size_t buflen = 32 + saltlen + passlen;
    uint8_t* buf = (uint8_t*)malloc(buflen);
    // first round: salt || password
    memcpy(buf, salt, saltlen);
    memcpy(buf + saltlen, password, passlen);
    sha256_bytes(buf, saltlen + passlen, out);
    for (int i=1;i<iterations;i++) {
        memcpy(buf, out, 32);
        memcpy(buf + 32, salt, saltlen);
        memcpy(buf + 32 + saltlen, password, passlen);
        sha256_bytes(buf, 32 + saltlen + passlen, out);
    }
    // zeroize
    memset(buf, 0, buflen);
    free(buf);
}

static void auth_init(Auth* a){
    memset(a, 0, sizeof(*a));
}

static int add_user(Auth* a, const char* username, const char* password){
    if (!username || !password || username[0] == '\0') return 0;
    for (int i=0;i<MAX_USERS;i++){
        if (a->users[i].used && strncmp(a->users[i].username, username, sizeof(a->users[i].username)) == 0) return 0;
    }
    for (int i=0;i<MAX_USERS;i++){
        if (!a->users[i].used){
            a->users[i].used = 1;
            strncpy(a->users[i].username, username, sizeof(a->users[i].username)-1);
            random_bytes(a->users[i].salt, sizeof(a->users[i].salt));
            derive_key(password, a->users[i].salt, sizeof(a->users[i].salt), 50000, a->users[i].hash);
            return 1;
        }
    }
    return 0;
}

static const char* login(Auth* a, const char* username, const char* password){
    if (!username || !password || username[0] == '\0') return "/login";
    for (int i=0;i<MAX_USERS;i++){
        if (a->users[i].used && strncmp(a->users[i].username, username, sizeof(a->users[i].username)) == 0){
            uint8_t computed[32];
            derive_key(password, a->users[i].salt, sizeof(a->users[i].salt), 50000, computed);
            int ok = ct_equal(computed, a->users[i].hash, 32);
            memset(computed, 0, sizeof(computed));
            return ok ? "/dashboard" : "/login";
        }
    }
    return "/login";
}

int main(void){
    Auth auth;
    auth_init(&auth);
    add_user(&auth, "alice", "Correct$Horse1");
    add_user(&auth, "bob", "S3curePass!");

    printf("%s\n", login(&auth, "alice", "Correct$Horse1")); // /dashboard
    printf("%s\n", login(&auth, "alice", "wrongpass"));      // /login
    printf("%s\n", login(&auth, "unknown", "whatever"));     // /login
    printf("%s\n", login(&auth, "", ""));                    // /login
    printf("%s\n", login(&auth, "bob", "S3curePass!"));      // /dashboard
    return 0;
}