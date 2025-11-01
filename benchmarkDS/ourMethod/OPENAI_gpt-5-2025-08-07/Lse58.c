#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Self-contained simple login system with PBKDF2-HMAC-SHA256 for password hashing

// ===== SHA-256 implementation =====
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static const uint32_t K256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x>>n) | (x<<(32-n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x>>3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x>>10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]){
    uint32_t m[64];
    for (int i=0;i<16;i++){
        m[i] = ((uint32_t)data[i*4]<<24) | ((uint32_t)data[i*4+1]<<16) | ((uint32_t)data[i*4+2]<<8) | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++){
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++){
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX* ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len){
    for (size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t out[32]){
    uint32_t i = (uint32_t)ctx->datalen;
    ctx->data[i++] = 0x80;
    if (i > 56){
        while (i<64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i<56) ctx->data[i++] = 0x00;
    ctx->bitlen += ctx->datalen * 8ull;
    for (int j=7;j>=0;j--){
        ctx->data[56 + (7-j)] = (uint8_t)((ctx->bitlen >> (j*8)) & 0xffu);
    }
    sha256_transform(ctx, ctx->data);
    for (int j=0;j<8;j++){
        out[j*4  ] = (uint8_t)((ctx->state[j] >> 24) & 0xffu);
        out[j*4+1] = (uint8_t)((ctx->state[j] >> 16) & 0xffu);
        out[j*4+2] = (uint8_t)((ctx->state[j] >> 8) & 0xffu);
        out[j*4+3] = (uint8_t)(ctx->state[j] & 0xffu);
    }
    memset(ctx, 0, sizeof(*ctx));
}

// ===== HMAC-SHA256 =====
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    const size_t block = 64;
    uint8_t k0[64];
    if (keylen > block) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, keylen);
        sha256_final(&c, k0);
        memset(k0+32, 0, 32);
    } else {
        memset(k0, 0, sizeof(k0));
        memcpy(k0, key, keylen);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i=0;i<64;i++){ ipad[i] = (uint8_t)(k0[i] ^ 0x36u); opad[i] = (uint8_t)(k0[i] ^ 0x5cu); }
    SHA256_CTX ci;
    sha256_init(&ci);
    sha256_update(&ci, ipad, 64);
    sha256_update(&ci, data, datalen);
    uint8_t inner[32];
    sha256_final(&ci, inner);

    SHA256_CTX co;
    sha256_init(&co);
    sha256_update(&co, opad, 64);
    sha256_update(&co, inner, 32);
    sha256_final(&co, out);

    memset(k0, 0, sizeof(k0));
    memset(ipad, 0, sizeof(ipad));
    memset(opad, 0, sizeof(opad));
    memset(inner, 0, sizeof(inner));
}

// ===== PBKDF2-HMAC-SHA256 =====
static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* dk, size_t dkLen){
    size_t hLen = 32;
    uint32_t blocks = (uint32_t)((dkLen + hLen - 1) / hLen);
    uint8_t* U = (uint8_t*)calloc(hLen, 1);
    uint8_t* T = (uint8_t*)calloc(hLen, 1);
    uint8_t* saltBlock = (uint8_t*)calloc(saltlen + 4, 1);
    if (!U || !T || !saltBlock) {
        if (U) { memset(U, 0, hLen); free(U); }
        if (T) { memset(T, 0, hLen); free(T); }
        if (saltBlock) { memset(saltBlock, 0, saltlen + 4); free(saltBlock); }
        return;
    }
    memcpy(saltBlock, salt, saltlen);

    for (uint32_t i=1; i<=blocks; i++){
        saltBlock[saltlen]   = (uint8_t)((i >> 24) & 0xffu);
        saltBlock[saltlen+1] = (uint8_t)((i >> 16) & 0xffu);
        saltBlock[saltlen+2] = (uint8_t)((i >> 8) & 0xffu);
        saltBlock[saltlen+3] = (uint8_t)(i & 0xffu);

        hmac_sha256(password, passlen, saltBlock, saltlen + 4, U);
        memcpy(T, U, hLen);
        for (uint32_t j=2; j<=iterations; j++){
            hmac_sha256(password, passlen, U, hLen, U);
            for (size_t k=0; k<hLen; k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i - 1) * hLen;
        size_t cp = (hLen < (dkLen - offset)) ? hLen : (dkLen - offset);
        memcpy(dk + offset, T, cp);
    }

    memset(U, 0, hLen); free(U);
    memset(T, 0, hLen); free(T);
    memset(saltBlock, 0, saltlen + 4); free(saltBlock);
}

static void secure_zero(void* v, size_t n){
    volatile uint8_t* p = (volatile uint8_t*)v;
    while (n--) *p++ = 0;
}

static int get_secure_random_bytes(uint8_t* out, size_t len){
    FILE* f = fopen("/dev/urandom", "rb");
    if (f){
        size_t r = fread(out, 1, len, f);
        fclose(f);
        if (r == len) return 1;
    }
    // fallback to rand (not cryptographically secure, but ensures program runs)
    for (size_t i=0;i<len;i++) out[i] = (uint8_t)(rand() & 0xFF);
    return 1;
}

static bool is_password_strong(const char* pw){
    if (!pw) return false;
    size_t n = strlen(pw);
    if (n < 12) return false;
    int u=0,l=0,d=0,s=0;
    for (size_t i=0;i<n;i++){
        char c = pw[i];
        if (c>='A'&&c<='Z') u=1;
        else if (c>='a'&&c<='z') l=1;
        else if (c>='0'&&c<='9') d=1;
        else s=1;
    }
    return u&&l&&d&&s;
}

static bool is_email_valid(const char* email){
    if (!email) return false;
    size_t n = strlen(email);
    if (n < 5 || n > 254) return false;
    const char* at = strchr(email, '@');
    if (!at || at==email || at==email + n - 1) return false;
    const char* dot = strchr(at+1, '.');
    if (!dot || dot == email + n - 1) return false;
    return true;
}

typedef struct {
    char* username;
    char* email;
    uint8_t salt[16];
    uint8_t hash[32];
    int has_hash;
    int logged_in;
} User;

static void user_init(User* u, const char* username, const char* email){
    u->username = strdup(username ? username : "");
    u->email = strdup(email ? email : "");
    memset(u->salt, 0, sizeof(u->salt));
    memset(u->hash, 0, sizeof(u->hash));
    u->has_hash = 0;
    u->logged_in = 0;
}

static void user_free(User* u){
    if (!u) return;
    if (u->username) { free(u->username); u->username = NULL; }
    if (u->email) { free(u->email); u->email = NULL; }
    secure_zero(u->salt, sizeof(u->salt));
    secure_zero(u->hash, sizeof(u->hash));
    u->has_hash = 0;
    u->logged_in = 0;
}

static bool set_password(User* u, const char* password){
    if (!u || !password) return false;
    if (!is_password_strong(password)) return false;
    if (!get_secure_random_bytes(u->salt, sizeof(u->salt))) return false;
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       u->salt, sizeof(u->salt),
                       210000, u->hash, sizeof(u->hash));
    u->has_hash = 1;
    return true;
}

static bool verify_password(const User* u, const char* password){
    if (!u || !password || !u->has_hash) return false;
    uint8_t derived[32];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       u->salt, sizeof(u->salt),
                       210000, derived, sizeof(derived));
    uint8_t diff = 0;
    for (size_t i=0;i<32;i++) diff |= (uint8_t)(derived[i] ^ u->hash[i]);
    secure_zero(derived, sizeof(derived));
    return diff == 0;
}

static bool login(User* u, const char* username, const char* password){
    if (!u || !username || !password) return false;
    if (strcmp(u->username, username) != 0) return false;
    if (verify_password(u, password)) {
        u->logged_in = 1;
        return true;
    }
    return false;
}

static bool change_email(User* u, const char* old_email, const char* new_email, const char* confirm_password){
    if (!u || !old_email || !new_email || !confirm_password) return false;
    if (!u->logged_in) return false;
    if (strcmp(u->email, old_email) != 0) return false;
    if (!is_email_valid(new_email)) return false;
    if (!verify_password(u, confirm_password)) return false;
    char* ne = strdup(new_email);
    if (!ne) return false;
    free(u->email);
    u->email = ne;
    return true;
}

static void generate_strong_password(char* out, size_t outsz){
    const char* upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* lower = "abcdefghijklmnopqrstuvwxyz";
    const char* digits = "0123456789";
    const char* special = "!@#$%^&*()-_=+[]{};:,.?/|";
    char all[256];
    snprintf(all, sizeof(all), "%s%s%s%s", upper, lower, digits, special);
    size_t len = 16;
    if (outsz < len + 1) len = outsz > 1 ? outsz - 1 : 0;
    if (len < 12) len = 12;
    if (len + 1 > outsz) len = outsz > 0 ? outsz - 1 : 0;
    if (len == 0) { if (outsz > 0) out[0] = '\0'; return; }

    // Ensure policy requirements
    out[0] = upper[rand()%26];
    out[1] = lower[rand()%26];
    out[2] = digits[rand()%10];
    out[3] = special[rand()%strlen(special)];
    size_t all_len = strlen(all);
    for (size_t i=4;i<len;i++){
        uint8_t b;
        get_secure_random_bytes(&b, 1);
        out[i] = all[b % all_len];
    }
    // shuffle
    for (size_t i=len-1;i>0;i--){
        uint8_t b;
        get_secure_random_bytes(&b, 1);
        size_t j = b % (i+1);
        char tmp = out[i]; out[i] = out[j]; out[j] = tmp;
    }
    out[len] = '\0';
}

int main(void){
    User user;
    user_init(&user, "alice", "alice@example.com");

    char password[64];
    generate_strong_password(password, sizeof(password));
    if (!set_password(&user, password)) {
        printf("Setup failed\n");
        user_free(&user);
        return 0;
    }

    // Test 1: Attempt change while not logged in -> false
    bool t1 = change_email(&user, "alice@example.com", "newalice@example.com", password);
    printf("Test1 change while not logged in: %s\n", t1 ? "true" : "false");

    // Test 2: Login with correct password -> true
    bool t2 = login(&user, "alice", password);
    printf("Test2 login with correct password: %s\n", t2 ? "true" : "false");

    // Test 3: Change with wrong old email -> false
    bool t3 = change_email(&user, "wrong@example.com", "newalice@example.com", password);
    printf("Test3 change with wrong old email: %s\n", t3 ? "true" : "false");

    // Test 4: Change with wrong password -> false
    bool t4 = change_email(&user, "alice@example.com", "newalice@example.com", "WrongPass!234");
    printf("Test4 change with wrong password: %s\n", t4 ? "true" : "false");

    // Test 5: Change with correct old email and password -> true
    bool t5 = change_email(&user, "alice@example.com", "newalice@example.com", password);
    printf("Test5 change with correct old email and password: %s\n", t5 ? "true" : "false");

    // Zeroize password and free
    secure_zero(password, sizeof(password));
    user_free(&user);
    return 0;
}