#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Minimal SHA-256 implementation in C
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static inline uint32_t rotr32(uint32_t x, uint32_t n){ return (x>>n) | (x<<(32-n)); }
static inline uint32_t ch32(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t maj32(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0_32(uint32_t x){ return rotr32(x,2) ^ rotr32(x,13) ^ rotr32(x,22); }
static inline uint32_t ep1_32(uint32_t x){ return rotr32(x,6) ^ rotr32(x,11) ^ rotr32(x,25); }
static inline uint32_t sig0_32(uint32_t x){ return rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3); }
static inline uint32_t sig1_32(uint32_t x){ return rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10); }

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
    uint32_t m[64];
    for (int i=0;i<16;i++){
        m[i] = (uint32_t)data[i*4]<<24 | (uint32_t)data[i*4+1]<<16 | (uint32_t)data[i*4+2]<<8 | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++){
        m[i] = sig1_32(m[i-2]) + m[i-7] + sig0_32(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++){
        uint32_t t1 = h + ep1_32(e) + ch32(e,f,g) + k256_c[i] + m[i];
        uint32_t t2 = ep0_32(a) + maj32(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a = t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init_c(SHA256_CTX *ctx) {
    ctx->datalen=0; ctx->bitlen=0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update_c(SHA256_CTX *ctx, const uint8_t *data, size_t len) {
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64){
            sha256_transform_c(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final_c(SHA256_CTX *ctx, uint8_t hash[32]) {
    size_t i = ctx->datalen;
    if (ctx->datalen < 56){
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform_c(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    for (int j=0;j<8;j++){
        ctx->data[63-j] = (uint8_t)((ctx->bitlen >> (j*8)) & 0xFF);
    }
    sha256_transform_c(ctx, ctx->data);
    for (i=0;i<4;i++){
        hash[i]      = (ctx->state[0] >> (24 - i*8)) & 0xFF;
        hash[i + 4]  = (ctx->state[1] >> (24 - i*8)) & 0xFF;
        hash[i + 8]  = (ctx->state[2] >> (24 - i*8)) & 0xFF;
        hash[i + 12] = (ctx->state[3] >> (24 - i*8)) & 0xFF;
        hash[i + 16] = (ctx->state[4] >> (24 - i*8)) & 0xFF;
        hash[i + 20] = (ctx->state[5] >> (24 - i*8)) & 0xFF;
        hash[i + 24] = (ctx->state[6] >> (24 - i*8)) & 0xFF;
        hash[i + 28] = (ctx->state[7] >> (24 - i*8)) & 0xFF;
    }
}

static void hmac_sha256_c(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t out[32]) {
    uint8_t k0[64], kipad[64], kopad[64], tk[32];
    if (key_len > 64) {
        SHA256_CTX c;
        sha256_init_c(&c);
        sha256_update_c(&c, key, key_len);
        sha256_final_c(&c, tk);
        memset(k0, 0, 64);
        memcpy(k0, tk, 32);
    } else {
        memset(k0, 0, 64);
        if (key_len > 0) memcpy(k0, key, key_len);
    }
    for (int i=0;i<64;i++){ kipad[i] = k0[i]^0x36; kopad[i] = k0[i]^0x5c; }
    SHA256_CTX ci;
    sha256_init_c(&ci);
    sha256_update_c(&ci, kipad, 64);
    if (data_len>0) sha256_update_c(&ci, data, data_len);
    uint8_t inner[32];
    sha256_final_c(&ci, inner);

    SHA256_CTX co;
    sha256_init_c(&co);
    sha256_update_c(&co, kopad, 64);
    sha256_update_c(&co, inner, 32);
    sha256_final_c(&co, out);

    memset(k0,0,sizeof(k0)); memset(kipad,0,sizeof(kipad)); memset(kopad,0,sizeof(kopad));
    memset(tk,0,sizeof(tk)); memset(inner,0,sizeof(inner));
}

static void pbkdf2_hmac_sha256_c(const uint8_t* password, size_t password_len,
                                 const uint8_t* salt, size_t salt_len,
                                 uint32_t iterations, uint8_t* out, size_t dkLen) {
    uint32_t l = (uint32_t)((dkLen + 31) / 32);
    uint32_t r = (uint32_t)(dkLen - (l - 1) * 32);
    uint8_t U[32], T[32];
    uint8_t* si = (uint8_t*)malloc(salt_len + 4);
    if (!si) return;
    memcpy(si, salt, salt_len);
    for (uint32_t i=1;i<=l;i++){
        si[salt_len+0] = (uint8_t)((i>>24)&0xFF);
        si[salt_len+1] = (uint8_t)((i>>16)&0xFF);
        si[salt_len+2] = (uint8_t)((i>>8)&0xFF);
        si[salt_len+3] = (uint8_t)(i&0xFF);
        hmac_sha256_c(password, password_len, si, salt_len+4, U);
        memcpy(T, U, 32);
        for (uint32_t j=1;j<iterations;j++){
            hmac_sha256_c(password, password_len, U, 32, U);
            for (int k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t to_copy = (i==l)? r : 32;
        memcpy(out + (i-1)*32, T, to_copy);
    }
    memset(U,0,sizeof(U)); memset(T,0,sizeof(T));
    memset(si,0,salt_len+4);
    free(si);
}

static void to_hex_c(const uint8_t* in, size_t n, char* out) {
    static const char* hex = "0123456789abcdef";
    for (size_t i=0;i<n;i++){
        out[2*i] = hex[(in[i]>>4)&0xF];
        out[2*i+1] = hex[in[i]&0xF];
    }
    out[2*n] = '\0';
}

static int from_hex_c(const char* hex, uint8_t* out, size_t out_len) {
    size_t len = strlen(hex);
    if (len % 2 != 0 || out_len < len/2) return 0;
    auto hexval = [](char c)->int{
        if (c>='0'&&c<='9') return c-'0';
        if (c>='a'&&c<='f') return c-'a'+10;
        if (c>='A'&&c<='F') return c-'A'+10;
        return -1;
    };
    for (size_t i=0;i<len/2;i++){
        int hi = hexval(hex[2*i]);
        int lo = hexval(hex[2*i+1]);
        if (hi<0||lo<0) return 0;
        out[i] = (uint8_t)((hi<<4)|lo);
    }
    return 1;
}

static int constant_time_eq_c(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t r = 0;
    for (size_t i=0;i<n;i++) r |= (a[i]^b[i]);
    return r == 0;
}

// Secure random bytes: try /dev/urandom
static int secure_random_bytes_c(uint8_t* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t r = fread(out, 1, len, f);
    fclose(f);
    return r == len;
}

#define ITERATIONS_C 210000
#define SALT_LEN_C 16
#define KEY_LEN_C 32

// In-memory "database"
typedef struct {
    char username[65];
    char hash[256];
} UserRec;
static UserRec USERS[128];
static size_t USER_COUNT = 0;

static int is_valid_username_c(const char* u) {
    size_t n = u ? strlen(u) : 0;
    if (n < 3 || n > 64) return 0;
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c=='_' || c=='.' || c=='-')) return 0;
    }
    return 1;
}
static int is_valid_password_c(const char* p) {
    size_t n = p ? strlen(p) : 0;
    if (n < 8) return 0;
    int lower=0, upper=0, digit=0, special=0;
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)p[i];
        if (islower(c)) lower=1;
        else if (isupper(c)) upper=1;
        else if (isdigit(c)) digit=1;
        else special=1;
    }
    return (lower + upper + digit + special) >= 3;
}

static int find_user_index_c(const char* username) {
    for (size_t i=0;i<USER_COUNT;i++){
        if (strncmp(USERS[i].username, username, 64) == 0) return (int)i;
    }
    return -1;
}

static int hash_password_c(const char* password, char* out, size_t out_size) {
    if (!password || !out || out_size < 128) return 0;
    uint8_t salt[SALT_LEN_C];
    if (!secure_random_bytes_c(salt, sizeof(salt))) return 0;
    uint8_t dk[KEY_LEN_C];
    pbkdf2_hmac_sha256_c((const uint8_t*)password, strlen(password), salt, sizeof(salt), ITERATIONS_C, dk, sizeof(dk));
    char salt_hex[SALT_LEN_C*2+1];
    char hash_hex[KEY_LEN_C*2+1];
    to_hex_c(salt, sizeof(salt), salt_hex);
    to_hex_c(dk, sizeof(dk), hash_hex);
    int written = snprintf(out, out_size, "pbkdf2_sha256:%u:%s:%s", (unsigned)ITERATIONS_C, salt_hex, hash_hex);
    memset(salt, 0, sizeof(salt));
    memset(dk, 0, sizeof(dk));
    return written > 0 && (size_t)written < out_size;
}

int update_user_password(const char* username, const char* new_password) {
    if (!is_valid_username_c(username) || !is_valid_password_c(new_password)) return 0;
    char hashbuf[256];
    if (!hash_password_c(new_password, hashbuf, sizeof(hashbuf))) return 0;
    int idx = find_user_index_c(username);
    if (idx >= 0) {
        strncpy(USERS[idx].hash, hashbuf, sizeof(USERS[idx].hash)-1);
        USERS[idx].hash[sizeof(USERS[idx].hash)-1] = '\0';
    } else {
        if (USER_COUNT >= (sizeof(USERS)/sizeof(USERS[0]))) return 0;
        strncpy(USERS[USER_COUNT].username, username, sizeof(USERS[USER_COUNT].username)-1);
        USERS[USER_COUNT].username[sizeof(USERS[USER_COUNT].username)-1] = '\0';
        strncpy(USERS[USER_COUNT].hash, hashbuf, sizeof(USERS[USER_COUNT].hash)-1);
        USERS[USER_COUNT].hash[sizeof(USERS[USER_COUNT].hash)-1] = '\0';
        USER_COUNT++;
    }
    return 1;
}

const char* get_stored_hash(const char* username) {
    int idx = find_user_index_c(username);
    if (idx < 0) return NULL;
    return USERS[idx].hash;
}

static int verify_password_hash_c(const char* stored, const char* password) {
    if (!stored || !password) return 0;
    // Parse "pbkdf2_sha256:iters:salt_hex:hash_hex"
    const char* p1 = strchr(stored, ':'); if (!p1) return 0;
    const char* p2 = strchr(p1+1, ':'); if (!p2) return 0;
    const char* p3 = strchr(p2+1, ':'); if (!p3) return 0;
    char alg[32]; size_t alg_len = (size_t)(p1 - stored);
    if (alg_len >= sizeof(alg)) return 0;
    memcpy(alg, stored, alg_len); alg[alg_len] = '\0';
    if (strcasecmp(alg, "pbkdf2_sha256") != 0) return 0;
    char iters_str[16];
    size_t iters_len = (size_t)(p2 - (p1+1));
    if (iters_len >= sizeof(iters_str)) return 0;
    memcpy(iters_str, p1+1, iters_len); iters_str[iters_len] = '\0';
    unsigned iters = (unsigned)strtoul(iters_str, NULL, 10);
    size_t salt_hex_len = (size_t)(p3 - (p2+1));
    char* salt_hex = (char*)malloc(salt_hex_len+1);
    if (!salt_hex) return 0;
    memcpy(salt_hex, p2+1, salt_hex_len); salt_hex[salt_hex_len] = '\0';
    const char* hash_hex = p3+1;

    size_t salt_len = salt_hex_len/2;
    uint8_t* salt = (uint8_t*)malloc(salt_len);
    if (!salt) { free(salt_hex); return 0; }
    if (!from_hex_c(salt_hex, salt, salt_len)) { free(salt_hex); free(salt); return 0; }
    size_t hash_len = strlen(hash_hex)/2;
    uint8_t* expected = (uint8_t*)malloc(hash_len);
    if (!expected) { free(salt_hex); free(salt); return 0; }
    if (!from_hex_c(hash_hex, expected, hash_len)) { free(salt_hex); free(salt); free(expected); return 0; }

    uint8_t* dk = (uint8_t*)malloc(hash_len);
    if (!dk) { free(salt_hex); free(salt); free(expected); return 0; }
    pbkdf2_hmac_sha256_c((const uint8_t*)password, strlen(password), salt, salt_len, iters, dk, hash_len);
    int ok = constant_time_eq_c(expected, dk, hash_len);

    memset(dk,0,hash_len);
    free(dk); free(expected); memset(salt,0,salt_len); free(salt); free(salt_hex);
    return ok;
}

int main(void) {
    struct { const char* u; const char* p; } tests[5] = {
        {"alice", "CorrectHorseBatteryStaple!"},
        {"bob_123", "s3cureP@ssw0rd"},
        {"charlie", "anotherStrong#Pass1"},
        {"invalid user", "weak pass"},
        {"dave", "short7!"}
    };
    for (int i=0;i<5;i++){
        printf("Update %s -> %s\n", tests[i].u, update_user_password(tests[i].u, tests[i].p) ? "true" : "false");
    }
    const char* alice_hash = get_stored_hash("alice");
    printf("Stored alice: %s\n", alice_hash ? "present" : "none");
    if (alice_hash) {
        printf("Verify alice correct: %s\n", verify_password_hash_c(alice_hash, "CorrectHorseBatteryStaple!") ? "true" : "false");
        printf("Verify alice wrong: %s\n", verify_password_hash_c(alice_hash, "WrongPassword!") ? "true" : "false");
    }
    printf("Stored invalid user: %s\n", get_stored_hash("invalid user") ? "present" : "none");
    return 0;
}