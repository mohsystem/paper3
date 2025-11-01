/* Chain-of-Through (comments only):
1) Implement login + email change requiring login and password confirmation.
2) Security: salted KDF via HMAC-SHA256 iterations; constant-time compare; validate input.
3) Secure coding: in-memory store; no secret logging.
4) Review: constant-time compare and validation checks.
5) Output: final code with 5 tests.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

// Minimal SHA-256 implementation
typedef uint8_t u8; typedef uint32_t u32; typedef uint64_t u64;

typedef struct {
    u32 state[8];
    u64 bitlen;
    u8 data[64];
    size_t datalen;
} SHA256Ctx;

static inline u32 rotr(u32 x, u32 n){ return (x>>n)|(x<<(32-n)); }
static inline u32 ch(u32 x,u32 y,u32 z){ return (x&y)^(~x&z); }
static inline u32 maj(u32 x,u32 y,u32 z){ return (x&y)^(x&z)^(y&z); }
static inline u32 e0(u32 x){ return rotr(x,2)^rotr(x,13)^rotr(x,22); }
static inline u32 e1(u32 x){ return rotr(x,6)^rotr(x,11)^rotr(x,25); }
static inline u32 s0(u32 x){ return rotr(x,7)^rotr(x,18)^(x>>3); }
static inline u32 s1(u32 x){ return rotr(x,17)^rotr(x,19)^(x>>10); }

static const u32 K256_C[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_init(SHA256Ctx *ctx){
    ctx->datalen = 0; ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_transform(SHA256Ctx *ctx, const u8 data[]){
    u32 m[64]; int i;
    for(i=0;i<16;i++){
        m[i] = (u32)data[i*4]<<24 | (u32)data[i*4+1]<<16 | (u32)data[i*4+2]<<8 | (u32)data[i*4+3];
    }
    for(i=16;i<64;i++){
        m[i] = s1(m[i-2]) + m[i-7] + s0(m[i-15]) + m[i-16];
    }
    u32 a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3],e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for(i=0;i<64;i++){
        u32 t1 = h + e1(e) + ch(e,f,g) + K256_C[i] + m[i];
        u32 t2 = e0(a) + maj(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a = t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(SHA256Ctx *ctx, const u8 data[], size_t len){
    size_t i;
    for(i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256Ctx *ctx, u8 hash[32]){
    u32 i = ctx->datalen;
    if(ctx->datalen < 56){
        ctx->data[i++] = 0x80;
        while(i<56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while(i<64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += (u64)ctx->datalen * 8;
    ctx->data[63] = (u8)(ctx->bitlen);
    ctx->data[62] = (u8)(ctx->bitlen>>8);
    ctx->data[61] = (u8)(ctx->bitlen>>16);
    ctx->data[60] = (u8)(ctx->bitlen>>24);
    ctx->data[59] = (u8)(ctx->bitlen>>32);
    ctx->data[58] = (u8)(ctx->bitlen>>40);
    ctx->data[57] = (u8)(ctx->bitlen>>48);
    ctx->data[56] = (u8)(ctx->bitlen>>56);
    sha256_transform(ctx, ctx->data);
    for(i=0;i<8;i++){
        hash[i*4]   = (ctx->state[i]>>24) & 0xff;
        hash[i*4+1] = (ctx->state[i]>>16) & 0xff;
        hash[i*4+2] = (ctx->state[i]>>8) & 0xff;
        hash[i*4+3] = (ctx->state[i]) & 0xff;
    }
}

static void hmac_sha256(const u8 *key, size_t keylen, const u8 *msg, size_t msglen, u8 out[32]){
    const size_t blockSize = 64;
    u8 k0[64]; u8 tk[32];
    if(keylen > blockSize){
        SHA256Ctx c; sha256_init(&c); sha256_update(&c, key, keylen); sha256_final(&c, tk);
        memset(k0, 0, sizeof(k0));
        memcpy(k0, tk, 32);
    } else {
        memset(k0, 0, sizeof(k0));
        memcpy(k0, key, keylen);
    }
    u8 o_key[64], i_key[64];
    for(size_t i=0;i<64;i++){ o_key[i] = k0[i]^0x5c; i_key[i] = k0[i]^0x36; }
    u8 inner[32];
    SHA256Ctx ci; sha256_init(&ci); sha256_update(&ci, i_key, 64); sha256_update(&ci, msg, msglen); sha256_final(&ci, inner);
    SHA256Ctx co; sha256_init(&co); sha256_update(&co, o_key, 64); sha256_update(&co, inner, 32); sha256_final(&co, out);
    // zeroize
    volatile u8 *vp = tk; for(size_t i=0;i<32;i++) vp[i]=0;
}

static void kdf_hmac_sha256(const char *password, const u8 *salt, size_t saltlen, int iterations, u8 out[32]){
    u8 tmp[32];
    hmac_sha256(salt, saltlen, (const u8*)password, strlen(password), tmp);
    for(int i=1;i<iterations;i++){
        hmac_sha256(salt, saltlen, tmp, 32, tmp);
    }
    memcpy(out, tmp, 32);
    volatile u8 *vp = tmp; for(size_t i=0;i<32;i++) vp[i]=0;
}

static int ct_equal(const u8 *a, const u8 *b, size_t len){
    unsigned char res = 0;
    for(size_t i=0;i<len;i++){
        res |= (a[i] ^ b[i]);
    }
    return res == 0;
}

static int strcasecmp_port(const char* a, const char* b){
    while(*a && *b){
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if(ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

typedef struct {
    char *username;
    char *email;
    u8 *salt; size_t saltlen;
    u8 pwdhash[32];
} User;

typedef struct {
    User **users;
    size_t count;
} UserStore;

static void urand(u8 *buf, size_t n){
    for(size_t i=0;i<n;i++){
        buf[i] = (u8)(rand() & 0xFF);
    }
}

static UserStore* store_new(){
    UserStore* s = (UserStore*)calloc(1, sizeof(UserStore));
    return s;
}

static void user_free(User* u){
    if(!u) return;
    free(u->username); free(u->email);
    if(u->salt){ volatile u8 *vp = u->salt; for(size_t i=0;i<u->saltlen;i++) vp[i]=0; free(u->salt); }
    volatile u8 *vp2 = u->pwdhash; for(size_t i=0;i<32;i++) vp2[i]=0;
    free(u);
}

static void store_free(UserStore* s){
    if(!s) return;
    for(size_t i=0;i<s->count;i++) user_free(s->users[i]);
    free(s->users);
    free(s);
}

static User* store_find_by_username(UserStore* s, const char* username){
    for(size_t i=0;i<s->count;i++){
        if(strcmp(s->users[i]->username, username)==0) return s->users[i];
    }
    return NULL;
}

static User* store_find_by_email(UserStore* s, const char* email){
    for(size_t i=0;i<s->count;i++){
        if(strcasecmp_port(s->users[i]->email, email)==0) return s->users[i];
    }
    return NULL;
}

static int store_register(UserStore* s, const char* username, const char* email, const char* password){
    if(!username || !email || !password) return 0;
    if(store_find_by_username(s, username)) return 0;
    if(store_find_by_email(s, email)) return 0;
    User* u = (User*)calloc(1, sizeof(User));
    u->username = strdup(username);
    u->email = strdup(email);
    u->saltlen = 16; u->salt = (u8*)malloc(u->saltlen);
    urand(u->salt, u->saltlen);
    kdf_hmac_sha256(password, u->salt, u->saltlen, 120000, u->pwdhash);
    s->users = (User**)realloc(s->users, (s->count+1)*sizeof(User*));
    s->users[s->count++] = u;
    return 1;
}

static int store_update_email(UserStore* s, User* u, const char* newEmail){
    if(!u || !newEmail) return 0;
    if(store_find_by_email(s, newEmail)) return 0;
    free(u->email);
    u->email = strdup(newEmail);
    return 1;
}

typedef struct {
    UserStore* store;
    User* currentUser;
} AuthService;

static AuthService* auth_new(UserStore* s){
    AuthService* a = (AuthService*)calloc(1, sizeof(AuthService));
    a->store = s; a->currentUser = NULL;
    return a;
}

static void auth_free(AuthService* a){ free(a); }

static int is_valid_email(const char* email){
    if(!email) return 0;
    size_t n = strlen(email);
    if(n==0 || n>254) return 0;
    // simple check: must contain one '@' and a dot after it.
    const char* at = strchr(email, '@');
    if(!at || at==email) return 0;
    const char* dot = strrchr(email, '.');
    if(!dot || dot<at+2 || dot==email+n-1) return 0;
    return 1;
}

static int auth_login(AuthService* a, const char* username, const char* password){
    if(!username || !password) return 0;
    User* u = store_find_by_username(a->store, username);
    if(!u) return 0;
    u8 out[32]; kdf_hmac_sha256(password, u->salt, u->saltlen, 120000, out);
    int ok = ct_equal(out, u->pwdhash, 32);
    volatile u8 *vp = out; for(size_t i=0;i<32;i++) vp[i]=0;
    if(ok){ a->currentUser = u; return 1; }
    return 0;
}

static void auth_logout(AuthService* a){ a->currentUser = NULL; }

static int auth_is_logged_in(AuthService* a){ return a->currentUser != NULL; }

static int auth_change_email(AuthService* a, const char* oldEmail, const char* newEmail, const char* confirmPassword){
    if(!auth_is_logged_in(a)) return 0;
    if(!oldEmail || !newEmail || !confirmPassword) return 0;
    if(!is_valid_email(oldEmail) || !is_valid_email(newEmail)) return 0;
    if(strcasecmp_port(a->currentUser->email, oldEmail)!=0) return 0;
    if(strcasecmp_port(oldEmail, newEmail)==0) return 0; // no-op disallowed
    // verify password
    u8 out[32]; kdf_hmac_sha256(confirmPassword, a->currentUser->salt, a->currentUser->saltlen, 120000, out);
    int passok = ct_equal(out, a->currentUser->pwdhash, 32);
    volatile u8 *vp = out; for(size_t i=0;i<32;i++) vp[i]=0;
    if(!passok) return 0;
    if(!store_find_by_email(a->store, oldEmail)) return 0;
    if(store_find_by_email(a->store, newEmail)) return 0;
    return store_update_email(a->store, a->currentUser, newEmail);
}

int main(){
    srand((unsigned)time(NULL));
    UserStore* store = store_new();
    AuthService* auth = auth_new(store);

    store_register(store, "alice", "alice@example.com", "StrongPass!1");
    store_register(store, "bob", "bob@example.com", "CorrectHorseBatteryStaple");

    // Test 1: success
    int t1login = auth_login(auth, "alice", "StrongPass!1");
    int t1change = auth_change_email(auth, "alice@example.com", "alice.new@example.com", "StrongPass!1");
    printf("Test1 success=%s\n", (t1login && t1change) ? "true" : "false");
    auth_logout(auth);

    // Test 2: change without login
    int t2 = auth_change_email(auth, "bob@example.com", "bob.new@example.com", "CorrectHorseBatteryStaple");
    printf("Test2 success=%s\n", (!t2) ? "true" : "false");

    // Test 3: wrong old email
    int t3login = auth_login(auth, "bob", "CorrectHorseBatteryStaple");
    int t3change = auth_change_email(auth, "wrongbob@example.com", "bob.new@example.com", "CorrectHorseBatteryStaple");
    printf("Test3 success=%s\n", (t3login && !t3change) ? "true" : "false");
    auth_logout(auth);

    // Test 4: wrong confirm password
    int t4login = auth_login(auth, "bob", "CorrectHorseBatteryStaple");
    int t4change = auth_change_email(auth, "bob@example.com", "bob.new@example.com", "wrongpassword");
    printf("Test4 success=%s\n", (t4login && !t4change) ? "true" : "false");
    auth_logout(auth);

    // Test 5: new email taken and invalid
    int t5login = auth_login(auth, "bob", "CorrectHorseBatteryStaple");
    int t5taken = auth_change_email(auth, "bob@example.com", "alice.new@example.com", "CorrectHorseBatteryStaple");
    int t5invalid = auth_change_email(auth, "bob@example.com", "not-an-email", "CorrectHorseBatteryStaple");
    printf("Test5 success=%s\n", (t5login && !t5taken && !t5invalid) ? "true" : "false");
    auth_logout(auth);

    auth_free(auth);
    store_free(store);
    return 0;
}