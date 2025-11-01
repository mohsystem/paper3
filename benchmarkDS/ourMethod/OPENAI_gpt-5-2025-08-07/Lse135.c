#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

#define SALT_LEN 16
#define KEY_LEN 32
#define ITERATIONS 210000u
#define MAX_USERS 16

// Secure zeroization
static void secure_memzero(void* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// ---- SHA-256 implementation ----
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static const uint32_t K256[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x>>n)|(x<<(32u-n)); }
static inline uint32_t ch(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x,uint32_t y,uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
static inline uint32_t ep1(uint32_t x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
static inline uint32_t sig0(uint32_t x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
static inline uint32_t sig1(uint32_t x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]){
    uint32_t m[64];
    for (uint32_t i=0,j=0;i<16;i++,j+=4){
        m[i]=(uint32_t)data[j]<<24|(uint32_t)data[j+1]<<16|(uint32_t)data[j+2]<<8|(uint32_t)data[j+3];
    }
    for (uint32_t i=16;i<64;i++){
        m[i]=sig1(m[i-2])+m[i-7]+sig0(m[i-15])+m[i-16];
    }
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3];
    uint32_t e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for (uint32_t i=0;i<64;i++){
        uint32_t t1=h+ep1(e)+ch(e,f,g)+K256[i]+m[i];
        uint32_t t2=ep0(a)+maj(a,b,c);
        h=g; g=f; f=e; e=d+t1;
        d=c; c=b; b=a; a=t1+t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX* ctx){
    ctx->datalen=0;
    ctx->bitlen=0;
    ctx->state[0]=0x6a09e667u; ctx->state[1]=0xbb67ae85u; ctx->state[2]=0x3c6ef372u; ctx->state[3]=0xa54ff53au;
    ctx->state[4]=0x510e527fu; ctx->state[5]=0x9b05688cu; ctx->state[6]=0x1f83d9abu; ctx->state[7]=0x5be0cd19u;
}

static void sha256_update(SHA256_CTX* ctx,const uint8_t* data,size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen]=data[i];
        ctx->datalen++;
        if (ctx->datalen==64){
            sha256_transform(ctx,ctx->data);
            ctx->bitlen+=512;
            ctx->datalen=0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    uint32_t i=ctx->datalen;
    ctx->data[i++]=0x80;
    if (i>56){
        while (i<64) ctx->data[i++]=0x00;
        sha256_transform(ctx,ctx->data);
        i=0;
    }
    while (i<56) ctx->data[i++]=0x00;

    ctx->bitlen += (uint64_t)ctx->datalen * 8u;
    ctx->data[63]=(uint8_t)(ctx->bitlen);
    ctx->data[62]=(uint8_t)(ctx->bitlen>>8);
    ctx->data[61]=(uint8_t)(ctx->bitlen>>16);
    ctx->data[60]=(uint8_t)(ctx->bitlen>>24);
    ctx->data[59]=(uint8_t)(ctx->bitlen>>32);
    ctx->data[58]=(uint8_t)(ctx->bitlen>>40);
    ctx->data[57]=(uint8_t)(ctx->bitlen>>48);
    ctx->data[56]=(uint8_t)(ctx->bitlen>>56);
    sha256_transform(ctx,ctx->data);

    for (i=0;i<4;i++){
        hash[i]=(uint8_t)((ctx->state[0]>>(24-i*8))&0xff);
        hash[i+4]=(uint8_t)((ctx->state[1]>>(24-i*8))&0xff);
        hash[i+8]=(uint8_t)((ctx->state[2]>>(24-i*8))&0xff);
        hash[i+12]=(uint8_t)((ctx->state[3]>>(24-i*8))&0xff);
        hash[i+16]=(uint8_t)((ctx->state[4]>>(24-i*8))&0xff);
        hash[i+20]=(uint8_t)((ctx->state[5]>>(24-i*8))&0xff);
        hash[i+24]=(uint8_t)((ctx->state[6]>>(24-i*8))&0xff);
        hash[i+28]=(uint8_t)((ctx->state[7]>>(24-i*8))&0xff);
    }
    secure_memzero(ctx, sizeof(*ctx));
}

// ---- HMAC-SHA256 ----
static void hmac_sha256(const uint8_t* key,size_t keylen,
                        const uint8_t* data,size_t datalen,
                        uint8_t out[32]){
    const size_t block=64;
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];
    if (keylen>block){
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c,key,keylen);
        sha256_final(&c,tk);
        key=tk; keylen=32;
    }
    memset(k_ipad,0x36,block);
    memset(k_opad,0x5c,block);
    for (size_t i=0;i<keylen;i++){
        k_ipad[i]^=key[i];
        k_opad[i]^=key[i];
    }
    uint8_t inner[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx,k_ipad,block);
    sha256_update(&ctx,data,datalen);
    sha256_final(&ctx,inner);

    sha256_init(&ctx);
    sha256_update(&ctx,k_opad,block);
    sha256_update(&ctx,inner,32);
    sha256_final(&ctx,out);

    secure_memzero(k_ipad,sizeof(k_ipad));
    secure_memzero(k_opad,sizeof(k_opad));
    secure_memzero(tk,sizeof(tk));
    secure_memzero(inner,sizeof(inner));
}

// ---- PBKDF2-HMAC-SHA256 ----
static void pbkdf2_hmac_sha256(const uint8_t* password,size_t passlen,
                               const uint8_t* salt,size_t saltlen,
                               uint32_t iterations,
                               uint8_t* out,size_t outlen){
    uint32_t blocks = (uint32_t)((outlen + 31u)/32u);
    uint8_t u[32], t[32];
    uint8_t* asalt = (uint8_t*)malloc(saltlen+4);
    if (!asalt) exit(1);
    memcpy(asalt, salt, saltlen);
    for (uint32_t i=1;i<=blocks;i++){
        asalt[saltlen+0]=(uint8_t)((i>>24)&0xff);
        asalt[saltlen+1]=(uint8_t)((i>>16)&0xff);
        asalt[saltlen+2]=(uint8_t)((i>>8)&0xff);
        asalt[saltlen+3]=(uint8_t)(i&0xff);
        hmac_sha256(password,passlen,asalt,saltlen+4,u);
        memcpy(t,u,32);
        for (uint32_t j=1;j<iterations;j++){
            hmac_sha256(password,passlen,u,32,u);
            for (size_t k=0;k<32;k++) t[k]^=u[k];
        }
        size_t off=(size_t)(i-1u)*32u;
        size_t clen = (outlen - off > 32) ? 32 : (outlen - off);
        memcpy(out+off, t, clen);
    }
    secure_memzero(u,sizeof(u));
    secure_memzero(t,sizeof(t));
    secure_memzero(asalt, saltlen+4);
    free(asalt);
}

// Constant-time compare
static int ct_equal(const uint8_t* a, const uint8_t* b, size_t n){
    uint8_t diff=0;
    for (size_t i=0;i<n;i++) diff |= (uint8_t)(a[i]^b[i]);
    return diff==0;
}

// RNG
static int get_random_bytes(uint8_t* out, size_t len){
#if defined(_WIN32)
    if (BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0) {
        return 1;
    }
    return 0;
#else
    FILE* f = fopen("/dev/urandom","rb");
    if (!f) return 0;
    size_t rd = fread(out,1,len,f);
    fclose(f);
    return rd==len;
#endif
}

// Username validation
static int valid_username(const char* u){
    if (!u) return 0;
    size_t n = strlen(u);
    if (n==0 || n>64) return 0;
    for (size_t i=0;i<n;i++){
        unsigned char c=(unsigned char)u[i];
        if (!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.')) return 0;
    }
    return 1;
}
static int valid_password_for_login(const char* p){
    if (!p) return 0;
    size_t n=strlen(p);
    return n>=1 && n<=1024;
}
static int password_strong(const char* p){
    if (!p) return 0;
    size_t n=strlen(p);
    if (n<12 || n>1024) return 0;
    int U=0,L=0,D=0,S=0;
    for (size_t i=0;i<n;i++){
        unsigned char c=(unsigned char)p[i];
        if (c>='A'&&c<='Z') U=1;
        else if (c>='a'&&c<='z') L=1;
        else if (c>='0'&&c<='9') D=1;
        else S=1;
    }
    return U&&L&&D&&S;
}

typedef struct {
    char username[65];
    uint8_t salt[SALT_LEN];
    uint8_t hash[KEY_LEN];
    uint32_t iterations;
    int used;
} UserRecord;

static UserRecord STORE[MAX_USERS];

static void register_user(const char* username, const char* password){
    if (!valid_username(username)) {
        fprintf(stderr, "Invalid username\n");
        exit(1);
    }
    if (!password_strong(password)) {
        fprintf(stderr, "Weak password\n");
        exit(1);
    }
    int idx=-1;
    for (int i=0;i<MAX_USERS;i++){
        if (!STORE[i].used){ idx=i; break; }
    }
    if (idx<0){
        fprintf(stderr, "User store full\n");
        exit(1);
    }
    memset(&STORE[idx], 0, sizeof(STORE[idx]));
    strncpy(STORE[idx].username, username, sizeof(STORE[idx].username)-1);
    if (!get_random_bytes(STORE[idx].salt, SALT_LEN)){
        fprintf(stderr, "RNG failure\n");
        exit(1);
    }
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       STORE[idx].salt, SALT_LEN, ITERATIONS,
                       STORE[idx].hash, KEY_LEN);
    STORE[idx].iterations = ITERATIONS;
    STORE[idx].used = 1;
}

static const char* check_mod(const char* username, const char* password){
    if (!valid_username(username) || !valid_password_for_login(password)) {
        return "false";
    }
    const UserRecord* rec=NULL;
    for (int i=0;i<MAX_USERS;i++){
        if (STORE[i].used && strcmp(STORE[i].username, username)==0){
            rec=&STORE[i];
            break;
        }
    }
    if (!rec) return "false";

    uint8_t dk[KEY_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       rec->salt, SALT_LEN, rec->iterations,
                       dk, KEY_LEN);
    int ok = ct_equal(dk, rec->hash, KEY_LEN);
    secure_memzero(dk, sizeof(dk));
    return ok ? "true" : "false";
}

int main(void){
    // Initialize users
    register_user("alice", "Str0ng!Pass#1");
    register_user("bob", "Another$Pass2");
    register_user("eve", "Complex%Pass3");

    // 5 test cases
    printf("%s\n", check_mod("alice", "Str0ng!Pass#1")); // true
    printf("%s\n", check_mod("alice", "wrong"));         // false
    printf("%s\n", check_mod("unknown", "whatever"));    // false
    printf("%s\n", check_mod("", "nope"));               // false
    printf("%s\n", check_mod("bob", "Another$Pass2"));   // true
    return 0;
}