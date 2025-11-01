/*
Chain-of-Through:
1) Understand: In-memory registration/login storing "hex(hash):hex(salt)".
2) Security: PBKDF2-HMAC-SHA256, secure random salt (/dev/urandom or getrandom), constant-time compare, input validation.
3) Implement securely without external dependencies.
4) Review: careful about memory and conversions.
5) Final secure output below.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(__linux__)
#include <sys/random.h>
#endif

// ---- SHA-256 implementation ----
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static inline uint32_t c_rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t c_ch(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t c_maj(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t c_ep0(uint32_t x){ return c_rotr(x,2) ^ c_rotr(x,13) ^ c_rotr(x,22); }
static inline uint32_t c_ep1(uint32_t x){ return c_rotr(x,6) ^ c_rotr(x,11) ^ c_rotr(x,25); }
static inline uint32_t c_sig0(uint32_t x){ return c_rotr(x,7) ^ c_rotr(x,18) ^ (x >> 3); }
static inline uint32_t c_sig1(uint32_t x){ return c_rotr(x,17) ^ c_rotr(x,19) ^ (x >> 10); }

static const uint32_t C_K256[64] = {
  0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
  0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
  0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
  0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
  0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
  0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
  0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
  0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static void c_sha256_transform(SHA256_CTX* ctx, const uint8_t data[]){
    uint32_t m[64];
    for(int i=0;i<16;i++){
        m[i] = ((uint32_t)data[i*4]<<24) | ((uint32_t)data[i*4+1]<<16) | ((uint32_t)data[i*4+2]<<8) | ((uint32_t)data[i*4+3]);
    }
    for(int i=16;i<64;i++){
        m[i] = c_sig1(m[i-2]) + m[i-7] + c_sig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3];
    uint32_t e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for(int i=0;i<64;i++){
        uint32_t t1 = h + c_ep1(e) + c_ch(e,f,g) + C_K256[i] + m[i];
        uint32_t t2 = c_ep0(a) + c_maj(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void c_sha256_init(SHA256_CTX* ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void c_sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len){
    for(size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen == 64){
            c_sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void c_sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    ctx->bitlen += ctx->datalen * 8ull;
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if(i > 56){
        while(i<64) ctx->data[i++] = 0x00;
        c_sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while(i<56) ctx->data[i++] = 0x00;
    uint64_t bl = ctx->bitlen;
    for(int j=7;j>=0;j--) ctx->data[i++] = (uint8_t)((bl >> (j*8)) & 0xff);
    c_sha256_transform(ctx, ctx->data);
    for(int j=0;j<8;j++){
        hash[j*4  ] = (uint8_t)((ctx->state[j] >> 24) & 0xff);
        hash[j*4+1] = (uint8_t)((ctx->state[j] >> 16) & 0xff);
        hash[j*4+2] = (uint8_t)((ctx->state[j] >> 8) & 0xff);
        hash[j*4+3] = (uint8_t)(ctx->state[j] & 0xff);
    }
}

// ---- HMAC-SHA256 ----
static void c_hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    uint8_t k_ipad[64], k_opad[64];
    uint8_t tk[32];
    const uint8_t* k = key;
    size_t kl = keylen;
    if(kl > 64){
        SHA256_CTX t;
        c_sha256_init(&t);
        c_sha256_update(&t, k, kl);
        c_sha256_final(&t, tk);
        k = tk; kl = 32;
    }
    memset(k_ipad, 0, 64);
    memset(k_opad, 0, 64);
    memcpy(k_ipad, k, kl);
    memcpy(k_opad, k, kl);
    for(int i=0;i<64;i++){ k_ipad[i] ^= 0x36; k_opad[i] ^= 0x5c; }
    SHA256_CTX c;
    c_sha256_init(&c);
    c_sha256_update(&c, k_ipad, 64);
    c_sha256_update(&c, data, datalen);
    uint8_t inner[32];
    c_sha256_final(&c, inner);

    c_sha256_init(&c);
    c_sha256_update(&c, k_opad, 64);
    c_sha256_update(&c, inner, 32);
    c_sha256_final(&c, out);

    memset(tk, 0, sizeof(tk));
    memset(inner, 0, sizeof(inner));
}

// ---- PBKDF2-HMAC-SHA256 ----
static void c_pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen, const uint8_t* salt, size_t saltlen, uint32_t iterations, uint8_t* dk, size_t dkLen){
    uint32_t blocks = (uint32_t)((dkLen + 31) / 32);
    uint8_t* asalt = (uint8_t*)malloc(saltlen + 4);
    if(!asalt) return;
    memcpy(asalt, salt, saltlen);
    uint8_t U[32], T[32];
    for(uint32_t i=1;i<=blocks;i++){
        asalt[saltlen+0] = (uint8_t)((i >> 24) & 0xff);
        asalt[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
        asalt[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
        asalt[saltlen+3] = (uint8_t)(i & 0xff);
        c_hmac_sha256(password, passlen, asalt, saltlen+4, U);
        memcpy(T, U, 32);
        for(uint32_t j=1;j<iterations;j++){
            c_hmac_sha256(password, passlen, U, 32, U);
            for(int k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i-1) * 32;
        size_t tocpy = (dkLen - offset > 32) ? 32 : (dkLen - offset);
        memcpy(dk + offset, T, tocpy);
    }
    memset(U, 0, sizeof(U));
    memset(T, 0, sizeof(T));
    memset(asalt, 0, saltlen+4);
    free(asalt);
}

static int const_time_eq(const uint8_t* a, const uint8_t* b, size_t len){
    uint8_t diff = 0;
    for(size_t i=0;i<len;i++) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

static char* to_hex(const uint8_t* data, size_t len){
    static const char* hex = "0123456789abcdef";
    char* s = (char*)malloc(len*2 + 1);
    if(!s) return NULL;
    for(size_t i=0;i<len;i++){
        s[2*i]   = hex[(data[i]>>4)&0xF];
        s[2*i+1] = hex[data[i]&0xF];
    }
    s[len*2] = '\0';
    return s;
}

static uint8_t* from_hex(const char* s, size_t* out_len){
    size_t n = strlen(s);
    if(n%2 != 0) return NULL;
    size_t len = n/2;
    uint8_t* out = (uint8_t*)malloc(len);
    if(!out) return NULL;
    for(size_t i=0;i<len;i++){
        char h = s[2*i], l = s[2*i+1];
        int hi = (h>='0'&&h<='9')?h-'0':(h>='a'&&h<='f')?h-'a'+10:(h>='A'&&h<='F')?h-'A'+10:-1;
        int lo = (l>='0'&&l<='9')?l-'0':(l>='a'&&l<='f')?l-'a'+10:(l>='A'&&l<='F')?l-'A'+10:-1;
        if(hi<0||lo<0){ free(out); return NULL; }
        out[i] = (uint8_t)((hi<<4)|lo);
    }
    *out_len = len;
    return out;
}

static int secure_random(uint8_t* buf, size_t len){
#if defined(__linux__)
    ssize_t r = getrandom(buf, len, 0);
    if(r == (ssize_t)len) return 1;
#endif
    FILE* f = fopen("/dev/urandom", "rb");
    if(!f) return 0;
    size_t rd = fread(buf, 1, len, f);
    fclose(f);
    return rd == len;
}

typedef struct {
    char* username;
    char* record; // "hex(hash):hex(salt)"
} Entry;

typedef struct {
    Entry* items;
    size_t count;
    size_t cap;
} Store;

static void store_init(Store* s){
    s->items = NULL; s->count = 0; s->cap = 0;
}
static void store_free(Store* s){
    for(size_t i=0;i<s->count;i++){
        free(s->items[i].username);
        free(s->items[i].record);
    }
    free(s->items);
    s->items = NULL; s->count=0; s->cap=0;
}
static Entry* store_find(Store* s, const char* username){
    for(size_t i=0;i<s->count;i++){
        if(strcmp(s->items[i].username, username)==0) return &s->items[i];
    }
    return NULL;
}
static int store_put(Store* s, const char* username, const char* record){
    if(store_find(s, username)) return 0;
    if(s->count == s->cap){
        size_t ncap = s->cap? s->cap*2 : 8;
        Entry* nitems = (Entry*)realloc(s->items, ncap * sizeof(Entry));
        if(!nitems) return 0;
        s->items = nitems; s->cap = ncap;
    }
    s->items[s->count].username = strdup(username);
    s->items[s->count].record = strdup(record);
    if(!s->items[s->count].username || !s->items[s->count].record){
        free(s->items[s->count].username);
        free(s->items[s->count].record);
        return 0;
    }
    s->count++;
    return 1;
}

static int valid_username(const char* u){
    size_t len = strlen(u);
    if(len < 3 || len > 64) return 0;
    for(size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)u[i];
        if(!(isalnum(c) || c=='_' || c=='.' || c=='-')) return 0;
    }
    return 1;
}

static int valid_password(const char* p){
    size_t len = strlen(p);
    if(len < 8 || len > 1024) return 0;
    for(size_t i=0;i<len;i++){
        unsigned char c = (unsigned char)p[i];
        if(c < 32) return 0;
    }
    return 1;
}

// Returns 1 on success, 0 on failure
static int register_user(Store* store, const char* username, const char* password){
    const uint32_t iterations = 120000;
    const size_t saltLen = 16;
    const size_t keyLen = 32;
    if(!valid_username(username) || !valid_password(password)) return 0;
    if(store_find(store, username)) return 0;

    uint8_t salt[saltLen];
    if(!secure_random(salt, saltLen)) return 0;

    uint8_t dk[keyLen];
    c_pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, saltLen, iterations, dk, keyLen);

    char* hhex = to_hex(dk, keyLen);
    char* shex = to_hex(salt, saltLen);
    if(!hhex || !shex){
        free(hhex); free(shex);
        return 0;
    }
    size_t rec_len = strlen(hhex) + 1 + strlen(shex) + 1;
    char* rec = (char*)malloc(rec_len);
    if(!rec){ free(hhex); free(shex); return 0; }
    snprintf(rec, rec_len, "%s:%s", hhex, shex);
    int ok = store_put(store, username, rec);
    free(hhex); free(shex); free(rec);
    // Clear dk
    memset(dk, 0, sizeof(dk));
    return ok;
}

// Returns 1 if verified, 0 otherwise
static int verify_login(Store* store, const char* username, const char* password){
    const uint32_t iterations = 120000;
    Entry* e = store_find(store, username);
    if(!e) return 0;
    const char* rec = e->record;
    const char* colon = strchr(rec, ':');
    if(!colon) return 0;
    size_t hlen = (size_t)(colon - rec);
    char* hhex = (char*)malloc(hlen + 1);
    if(!hhex) return 0;
    memcpy(hhex, rec, hlen);
    hhex[hlen] = '\0';
    const char* shex = colon + 1;

    size_t stored_len=0, salt_len=0;
    uint8_t* stored = from_hex(hhex, &stored_len);
    uint8_t* salt = from_hex(shex, &salt_len);
    free(hhex);
    if(!stored || !salt || stored_len == 0){
        free(stored); free(salt);
        return 0;
    }
    uint8_t* dk = (uint8_t*)malloc(stored_len);
    if(!dk){ free(stored); free(salt); return 0; }
    c_pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, salt_len, iterations, dk, stored_len);
    int ok = const_time_eq(stored, dk, stored_len);
    memset(dk, 0, stored_len);
    free(dk); free(stored); free(salt);
    return ok;
}

int main(void){
    Store store;
    store_init(&store);

    // Test case 1: Register new user
    printf("TC1 register alice: %s\n", register_user(&store, "alice", "S3curePwd!") ? "true" : "false");

    // Test case 2: Duplicate registration
    printf("TC2 register alice again: %s\n", register_user(&store, "alice", "AnotherPwd!") ? "true" : "false");

    // Test case 3: Correct login
    printf("TC3 login alice correct: %s\n", verify_login(&store, "alice", "S3curePwd!") ? "true" : "false");

    // Test case 4: Wrong password
    printf("TC4 login alice wrong: %s\n", verify_login(&store, "alice", "WrongPass!") ? "true" : "false");

    // Test case 5: Another user register and login
    printf("TC5 register bob: %s\n", register_user(&store, "bob-01", "P@ssw0rd123") ? "true" : "false");
    printf("TC5 login bob correct: %s\n", verify_login(&store, "bob-01", "P@ssw0rd123") ? "true" : "false");

    store_free(&store);
    return 0;
}