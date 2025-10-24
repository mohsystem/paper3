#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

/* Minimal SHA-256 implementation */
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    u32 state[8];
    u64 bitlen;
    u8 data[64];
    size_t datalen;
} SHA256_CTX;

static inline u32 ROTR(u32 x, u32 n){ return (x >> n) | (x << (32 - n)); }
static inline u32 CH(u32 x, u32 y, u32 z){ return (x & y) ^ (~x & z); }
static inline u32 MAJ(u32 x, u32 y, u32 z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline u32 EP0(u32 x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline u32 EP1(u32 x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline u32 SIG0(u32 x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline u32 SIG1(u32 x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static const u32 K256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_init(SHA256_CTX* ctx){
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
    ctx->bitlen = 0;
    ctx->datalen = 0;
}

static void sha256_transform(SHA256_CTX* ctx, const u8 data[64]){
    u32 m[64];
    for (int i=0;i<16;i++){
        m[i] = ((u32)data[i*4]<<24) | ((u32)data[i*4+1]<<16) | ((u32)data[i*4+2]<<8) | (u32)data[i*4+3];
    }
    for (int i=16;i<64;i++){
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    u32 a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    u32 e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++){
        u32 t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        u32 t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(SHA256_CTX* ctx, const u8* in, size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = in[i];
        if (ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, u8 out[32]){
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if (i > 56){
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;
    ctx->bitlen += ctx->datalen * 8ULL;
    for (int j=7;j>=0;j--) {
        ctx->data[i++] = (u8)((ctx->bitlen >> (j*8)) & 0xff);
    }
    sha256_transform(ctx, ctx->data);
    for (int j=0;j<8;j++){
        out[j*4+0] = (u8)((ctx->state[j] >> 24) & 0xff);
        out[j*4+1] = (u8)((ctx->state[j] >> 16) & 0xff);
        out[j*4+2] = (u8)((ctx->state[j] >> 8) & 0xff);
        out[j*4+3] = (u8)((ctx->state[j] >> 0) & 0xff);
    }
}

static void bytes_to_hex(const u8* in, size_t n, char* out){
    static const char* hex = "0123456789abcdef";
    for (size_t i=0;i<n;i++){
        out[i*2]   = hex[(in[i]>>4)&0xF];
        out[i*2+1] = hex[in[i]&0xF];
    }
    out[n*2] = '\0';
}
static int hex_to_bytes(const char* hex, u8* out, size_t outcap){
    size_t len = strlen(hex);
    if (len % 2 != 0) return -1;
    size_t n = len/2;
    if (n > outcap) return -1;
    for (size_t i=0;i<n;i++){
        char h = hex[i*2], l = hex[i*2+1];
        int hv = (h>='0'&&h<='9')? h-'0' : (h>='a'&&h<='f')? h-'a'+10 : (h>='A'&&h<='F')? h-'A'+10 : -1;
        int lv = (l>='0'&&l<='9')? l-'0' : (l>='a'&&l<='f')? l-'a'+10 : (l>='A'&&l<='F')? l-'A'+10 : -1;
        if (hv<0 || lv<0) return -1;
        out[i] = (u8)((hv<<4)|lv);
    }
    return (int)n;
}

static void sha256_hex_with_salt(const char* password, const u8* salt, size_t saltsz, char out_hex[65]){
    SHA256_CTX ctx; sha256_init(&ctx);
    sha256_update(&ctx, salt, saltsz);
    sha256_update(&ctx, (const u8*)password, strlen(password));
    u8 out[32]; sha256_final(&ctx, out);
    bytes_to_hex(out, 32, out_hex);
}

/* Simple in-memory store */
#define MAX_USERS 100
#define USERNAME_MAX 64
#define STORED_MAX 200

typedef struct {
    char username[USERNAME_MAX];
    char stored[STORED_MAX]; // saltHex:hashHex
    int used;
} Entry;

typedef struct {
    Entry entries[MAX_USERS];
} UserStore;

void userstore_init(UserStore* us){
    for (int i=0;i<MAX_USERS;i++) us->entries[i].used = 0;
    srand((unsigned)time(NULL));
}

static int find_user_index(UserStore* us, const char* username){
    for (int i=0;i<MAX_USERS;i++){
        if (us->entries[i].used && strcmp(us->entries[i].username, username)==0) return i;
    }
    return -1;
}

int signup(UserStore* us, const char* username, const char* password){
    if (!us || !username || !password) return 0;
    if (find_user_index(us, username) >= 0) return 0;
    int slot = -1;
    for (int i=0;i<MAX_USERS;i++){ if (!us->entries[i].used){ slot = i; break; } }
    if (slot < 0) return 0;
    u8 salt[16];
    for (int i=0;i<16;i++) salt[i] = (u8)(rand() & 0xFF);
    char saltHex[33]; bytes_to_hex(salt, 16, saltHex);
    char hashHex[65]; sha256_hex_with_salt(password, salt, 16, hashHex);
    snprintf(us->entries[slot].username, USERNAME_MAX, "%s", username);
    snprintf(us->entries[slot].stored, STORED_MAX, "%s:%s", saltHex, hashHex);
    us->entries[slot].used = 1;
    return 1;
}

int verify(UserStore* us, const char* username, const char* password){
    int idx = find_user_index(us, username);
    if (idx < 0 || !password) return 0;
    char* sep = strchr(us->entries[idx].stored, ':');
    if (!sep) return 0;
    char saltHex[65];
    size_t saltLen = (size_t)(sep - us->entries[idx].stored);
    if (saltLen >= sizeof(saltHex)) return 0;
    memcpy(saltHex, us->entries[idx].stored, saltLen);
    saltHex[saltLen] = '\0';
    u8 salt[64];
    int saltBytes = hex_to_bytes(saltHex, salt, sizeof(salt));
    if (saltBytes <= 0) return 0;
    char expected[65];
    snprintf(expected, sizeof(expected), "%s", sep+1);
    char actual[65];
    sha256_hex_with_salt(password, salt, (size_t)saltBytes, actual);
    if (strlen(expected) != strlen(actual)) return 0;
    unsigned diff = 0;
    for (size_t i=0;i<strlen(expected);i++) diff |= (unsigned char)expected[i] ^ (unsigned char)actual[i];
    return diff == 0;
}

const char* get_stored(UserStore* us, const char* username){
    int idx = find_user_index(us, username);
    if (idx < 0) return NULL;
    return us->entries[idx].stored;
}

int main(){
    UserStore us; userstore_init(&us);
    printf("Test1 signup alice: %s\n", signup(&us, "alice", "Password123!") ? "true" : "false");
    printf("Test2 signup bob: %s\n", signup(&us, "bob", "Secr3t!") ? "true" : "false");
    printf("Test3 duplicate alice: %s\n", signup(&us, "alice", "another") ? "true" : "false");
    printf("Test4 verify bob correct: %s\n", verify(&us, "bob", "Secr3t!") ? "true" : "false");
    printf("Test5 verify bob wrong: %s\n", verify(&us, "bob", "wrong") ? "true" : "false");
    const char* stored = get_stored(&us, "alice");
    printf("Stored alice: %s\n", stored ? stored : "(null)");
    return 0;
}