#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Minimal SHA-256 implementation */
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} sha256_ctx;

static inline uint32_t rotr32(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x){ return rotr32(x,2) ^ rotr32(x,13) ^ rotr32(x,22); }
static inline uint32_t ep1(uint32_t x){ return rotr32(x,6) ^ rotr32(x,11) ^ rotr32(x,25); }
static inline uint32_t sig0(uint32_t x){ return rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x){ return rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10); }

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

static void sha256_init(sha256_ctx* ctx){
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
    ctx->bitlen=0; ctx->datalen=0;
}

static void sha256_transform(sha256_ctx* ctx, const uint8_t data[64]) {
    uint32_t m[64];
    for (int i=0;i<16;i++){
        m[i]=(uint32_t)data[i*4]<<24|(uint32_t)data[i*4+1]<<16|(uint32_t)data[i*4+2]<<8|(uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++) m[i]=sig1(m[i-2])+m[i-7]+sig0(m[i-15])+m[i-16];
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3],e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for (int i=0;i<64;i++){
        uint32_t t1=h+ep1(e)+ch(e,f,g)+K256[i]+m[i];
        uint32_t t2=ep0(a)+maj(a,b,c);
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(sha256_ctx* ctx, const uint8_t* data, size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen==64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen=0;
        }
    }
}

static void sha256_final(sha256_ctx* ctx, uint8_t hash[32]){
    size_t i=ctx->datalen;
    ctx->data[i++]=0x80;
    if (i>56){
        while (i<64) ctx->data[i++]=0x00;
        sha256_transform(ctx, ctx->data);
        i=0;
    }
    while (i<56) ctx->data[i++]=0x00;
    ctx->bitlen += ctx->datalen*8ULL;
    for (int j=7;j>=0;--j) ctx->data[i++] = (uint8_t)((ctx->bitlen>>(j*8))&0xFF);
    sha256_transform(ctx, ctx->data);
    for (int j=0;j<8;j++){
        hash[j*4]  =(uint8_t)((ctx->state[j]>>24)&0xFF);
        hash[j*4+1]=(uint8_t)((ctx->state[j]>>16)&0xFF);
        hash[j*4+2]=(uint8_t)((ctx->state[j]>>8)&0xFF);
        hash[j*4+3]=(uint8_t)(ctx->state[j]&0xFF);
    }
}

static void sha256_bytes(const uint8_t* data, size_t len, uint8_t out[32]){
    sha256_ctx c; sha256_init(&c); sha256_update(&c, data, len); sha256_final(&c, out);
}

// Derive salt deterministically from username (for demo; use random salt in production)
static void derive_salt_from_username(const char* username, uint8_t salt_out[16]){
    const char* prefix = "salt:";
    size_t plen = strlen(prefix), ulen = strlen(username);
    size_t len = plen + ulen;
    uint8_t* buf = (uint8_t*)malloc(len);
    memcpy(buf, prefix, plen);
    memcpy(buf + plen, username, ulen);
    uint8_t h[32];
    sha256_bytes(buf, len, h);
    memcpy(salt_out, h, 16);
    free(buf);
}

// Simple iterative salted SHA-256 KDF (for demo)
static void kdf_iter_sha256(const char* password, const uint8_t* salt, uint32_t iterations, uint8_t out[32]){
    size_t passlen = strlen(password);
    uint8_t* init = (uint8_t*)malloc(16 + passlen);
    memcpy(init, salt, 16);
    memcpy(init + 16, password, passlen);
    sha256_bytes(init, 16 + passlen, out);
    free(init);
    uint8_t tmp[32];
    memcpy(tmp, out, 32);
    for (uint32_t i=1;i<iterations;i++){
        sha256_bytes(tmp, 32, out);
        memcpy(tmp, out, 32);
    }
}

// Constant-time compare
static int ct_equal(const uint8_t* a, const uint8_t* b, size_t n){
    uint8_t acc=0;
    for (size_t i=0;i<n;i++) acc |= (uint8_t)(a[i]^b[i]);
    return acc==0;
}

typedef struct {
    char username[64];
    uint8_t salt[16];
    uint8_t hash[32];
    uint32_t iterations;
    int in_use;
} user_record;

#define MAX_USERS 32

typedef struct {
    user_record users[MAX_USERS];
    uint32_t iterations;
} auth_db;

static void db_init(auth_db* db){
    memset(db, 0, sizeof(*db));
    db->iterations = 100000;
}

static void db_add_user(auth_db* db, const char* username, const char* password){
    if (!username || !password || username[0]=='\0') return;
    for (int i=0;i<MAX_USERS;i++){
        if (!db->users[i].in_use){
            strncpy(db->users[i].username, username, sizeof(db->users[i].username)-1);
            db->users[i].username[sizeof(db->users[i].username)-1]='\0';
            derive_salt_from_username(username, db->users[i].salt);
            kdf_iter_sha256(password, db->users[i].salt, db->iterations, db->users[i].hash);
            db->users[i].iterations = db->iterations;
            db->users[i].in_use = 1;
            return;
        }
    }
}

static const char* db_login(auth_db* db, const char* username, const char* password){
    if (!username || !password) return NULL;
    for (int i=0;i<MAX_USERS;i++){
        if (db->users[i].in_use && strcmp(db->users[i].username, username)==0){
            uint8_t out[32];
            kdf_iter_sha256(password, db->users[i].salt, db->users[i].iterations, out);
            if (ct_equal(out, db->users[i].hash, 32)) return "/home";
            return NULL;
        }
    }
    return NULL;
}

int main(void){
    auth_db db;
    db_init(&db);
    db_add_user(&db, "alice", "Password123!");
    db_add_user(&db, "bob", "s3cur3");

    const char* users[5] = {"alice","alice","bob","charlie",""};
    const char* passes[5] = {"Password123!","wrong","s3cur3","anything",""};

    for (int i=0;i<5;i++){
        const char* res = db_login(&db, users[i], passes[i]);
        printf("Test %d: %s\n", i+1, res ? res : "login failed");
    }
    return 0;
}