/*
Chain-of-Through in comments:
1) Problem understanding: Register a user by hashing password and inserting into a database (here, an in-memory store).
2) Security requirements: Validate inputs; use PBKDF2-HMAC-SHA256 with random salt and sufficient iterations; avoid plaintext storage.
3) Secure coding generation: Implement SHA-256, HMAC, PBKDF2; use /dev/urandom for randomness; careful memory handling.
4) Code review: No plaintext persistence; bounds checks; uniqueness enforcement.
5) Secure code output: Final implementation below.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

typedef uint8_t u8; typedef uint32_t u32; typedef uint64_t u64;

/* SHA-256 */
typedef struct {
    u32 state[8];
    u64 bitlen;
    u8 data[64];
    size_t datalen;
} sha256_ctx;

static inline u32 ROTR(u32 x, u32 n){ return (x>>n)|(x<<(32-n)); }
static inline u32 CH(u32 x,u32 y,u32 z){ return (x&y) ^ (~x&z); }
static inline u32 MAJ(u32 x,u32 y,u32 z){ return (x&y) ^ (x&z) ^ (y&z); }
static inline u32 EP0(u32 x){ return ROTR(x,2)^ROTR(x,13)^ROTR(x,22); }
static inline u32 EP1(u32 x){ return ROTR(x,6)^ROTR(x,11)^ROTR(x,25); }
static inline u32 SIG0(u32 x){ return ROTR(x,7)^ROTR(x,18)^(x>>3); }
static inline u32 SIG1(u32 x){ return ROTR(x,17)^ROTR(x,19)^(x>>10); }

static const u32 K256[64]={
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(sha256_ctx* ctx, const u8 data[64]){
    u32 m[64];
    for(int i=0;i<16;i++){
        m[i] = ((u32)data[i*4]<<24)|((u32)data[i*4+1]<<16)|((u32)data[i*4+2]<<8)|((u32)data[i*4+3]);
    }
    for(int i=16;i<64;i++){
        m[i]=SIG1(m[i-2])+m[i-7]+SIG0(m[i-15])+m[i-16];
    }
    u32 a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3],e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for(int i=0;i<64;i++){
        u32 t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        u32 t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(sha256_ctx* ctx){
    ctx->datalen=0; ctx->bitlen=0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update(sha256_ctx* ctx, const u8* data, size_t len){
    for(size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen==64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(sha256_ctx* ctx, u8 hash[32]){
    u64 bitlen_total = ctx->bitlen + ctx->datalen*8;
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if(i>56){
        while(i<64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i=0;
    }
    while(i<56) ctx->data[i++] = 0x00;
    for(int j=7;j>=0;j--){
        ctx->data[i++] = (u8)((bitlen_total >> (j*8)) & 0xff);
    }
    sha256_transform(ctx, ctx->data);
    for(i=0;i<8;i++){
        hash[i*4+0]=(u8)((ctx->state[i]>>24)&0xff);
        hash[i*4+1]=(u8)((ctx->state[i]>>16)&0xff);
        hash[i*4+2]=(u8)((ctx->state[i]>>8)&0xff);
        hash[i*4+3]=(u8)(ctx->state[i]&0xff);
    }
}

/* HMAC-SHA256 */
static void hmac_sha256(const u8* key, size_t keylen, const u8* data, size_t datalen, u8 out[32]){
    const size_t block=64;
    u8 k[64]; memset(k,0,sizeof(k));
    if(keylen > block){
        sha256_ctx c; sha256_init(&c); sha256_update(&c, key, keylen); sha256_final(&c, k);
    } else {
        memcpy(k, key, keylen);
    }
    u8 ipad[64], opad[64];
    for(size_t i=0;i<64;i++){ ipad[i]=k[i]^0x36; opad[i]=k[i]^0x5c; }
    sha256_ctx c;
    sha256_init(&c); sha256_update(&c, ipad, 64); sha256_update(&c, data, datalen);
    u8 inner[32]; sha256_final(&c, inner);
    sha256_init(&c); sha256_update(&c, opad, 64); sha256_update(&c, inner, 32);
    sha256_final(&c, out);
    memset(k,0,sizeof(k)); memset(ipad,0,sizeof(ipad)); memset(opad,0,sizeof(opad)); memset(inner,0,sizeof(inner));
}

/* PBKDF2-HMAC-SHA256 */
static void pbkdf2_hmac_sha256(const u8* password, size_t passlen, const u8* salt, size_t saltlen, uint32_t iterations, u8* out, size_t dkLen){
    uint32_t blocks = (uint32_t)((dkLen + 31)/32);
    u8* sb = (u8*)malloc(saltlen + 4);
    if(!sb) return;
    memcpy(sb, salt, saltlen);
    size_t offset=0;
    for(uint32_t i=1;i<=blocks;i++){
        sb[saltlen+0]=(u8)((i>>24)&0xff);
        sb[saltlen+1]=(u8)((i>>16)&0xff);
        sb[saltlen+2]=(u8)((i>>8)&0xff);
        sb[saltlen+3]=(u8)(i&0xff);
        u8 u[32], t[32];
        hmac_sha256(password, passlen, sb, saltlen+4, u);
        memcpy(t, u, 32);
        for(uint32_t j=1;j<iterations;j++){
            hmac_sha256(password, passlen, u, 32, u);
            for(int k=0;k<32;k++) t[k]^=u[k];
        }
        size_t clen = (dkLen - offset) < 32 ? (dkLen - offset) : 32;
        memcpy(out+offset, t, clen);
        offset += clen;
        memset(u,0,sizeof(u)); memset(t,0,sizeof(t));
    }
    memset(sb,0,saltlen+4); free(sb);
}

/* secure random: /dev/urandom */
static int secure_random(u8* buf, size_t len){
    FILE* f = fopen("/dev/urandom","rb");
    if(!f) return 0;
    size_t r = fread(buf,1,len,f);
    fclose(f);
    return r==len;
}

static int valid_username(const char* u){
    size_t n = strlen(u);
    if(n<3 || n>64) return 0;
    for(size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)u[i];
        if(!(isalnum(c) || c=='_' || c=='.' || c=='-')) return 0;
    }
    return 1;
}

static int valid_password(const char* p){
    size_t n = strlen(p);
    if(n<8 || n>1024) return 0;
    int U=0,L=0,D=0,S=0;
    for(size_t i=0;i<n;i++){
        unsigned char c=(unsigned char)p[i];
        if(isupper(c)) U=1;
        else if(islower(c)) L=1;
        else if(isdigit(c)) D=1;
        else S=1;
    }
    return U&&L&&D&&S;
}

/* In-memory DB */
typedef struct {
    int id;
    char* username;
    u8 salt[16];
    u8 hash[32];
} User;

static User* DB = NULL;
static size_t DB_SIZE = 0;
static size_t DB_CAP = 0;
static int NEXT_ID = 1;

static void db_free(){
    for(size_t i=0;i<DB_SIZE;i++){
        free(DB[i].username);
    }
    free(DB);
    DB=NULL; DB_SIZE=DB_CAP=0;
}

static void hexenc(const u8* in, size_t len, char* out){
    static const char* H="0123456789abcdef";
    for(size_t i=0;i<len;i++){
        out[i*2+0]=H[in[i]>>4];
        out[i*2+1]=H[in[i]&0xF];
    }
    out[len*2]='\0';
}

int register_user(const char* username, const char* password){
    if(!valid_username(username) || !valid_password(password)) return 0;
    // uniqueness
    for(size_t i=0;i<DB_SIZE;i++){
        if(strcmp(DB[i].username, username)==0) return 0;
    }
    u8 salt[16];
    if(!secure_random(salt, sizeof(salt))) return 0;

    u8 dk[32];
    pbkdf2_hmac_sha256((const u8*)password, strlen(password), salt, sizeof(salt), 200000, dk, sizeof(dk));

    // grow DB
    if(DB_SIZE==DB_CAP){
        size_t ncap = DB_CAP? DB_CAP*2 : 8;
        User* nd = (User*)realloc(DB, ncap*sizeof(User));
        if(!nd) return 0;
        DB = nd; DB_CAP = ncap;
    }
    User* u = &DB[DB_SIZE];
    u->id = NEXT_ID++;
    u->username = (char*)malloc(strlen(username)+1);
    if(!u->username) return 0;
    strcpy(u->username, username);
    memcpy(u->salt, salt, sizeof(salt));
    memcpy(u->hash, dk, sizeof(dk));
    DB_SIZE++;

    // wipe sensitive locals
    memset(salt,0,sizeof(salt));
    memset(dk,0,sizeof(dk));
    return 1;
}

int main(){
    // 5 test cases
    printf("T1 alice: %s\n", register_user("alice","StrongP@ssw0rd!") ? "true":"false");
    printf("T2 bob (weak): %s\n", register_user("bob","weak") ? "true":"false");
    printf("T3 alice duplicate: %s\n", register_user("alice","AnotherStr0ng!") ? "true":"false");
    printf("T4 charlie: %s\n", register_user("charlie","Cmplx#Pass123") ? "true":"false");
    printf("T5 bad username: %s\n", register_user("bad user","G00d#Pass!") ? "true":"false");

    // Show stored users
    for(size_t i=0;i<DB_SIZE;i++){
        char sh[16*2+1], hh[32*2+1];
        hexenc(DB[i].salt, 16, sh);
        hexenc(DB[i].hash, 32, hh);
        printf("id=%d, user=%s, salt=%s, hash=%s\n", DB[i].id, DB[i].username, sh, hh);
    }

    db_free();
    return 0;
}