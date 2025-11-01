#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Minimal SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256 implementation in C

typedef struct {
    uint32_t state[8];
    uint64_t bitcount;
    uint8_t buffer[64];
} sha256_ctx;

static inline uint32_t rotr32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

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

static void secure_memzero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

static void sha256_init(sha256_ctx* ctx) {
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
    ctx->bitcount = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

static void sha256_transform(sha256_ctx* ctx, const uint8_t data[64]) {
    uint32_t w[64];
    for (int i=0;i<16;i++) {
        w[i] = ((uint32_t)data[i*4]<<24) | ((uint32_t)data[i*4+1]<<16) | ((uint32_t)data[i*4+2]<<8) | ((uint32_t)data[i*4+3]);
    }
    for (int i=16;i<64;i++) {
        uint32_t s0 = rotr32(w[i-15],7) ^ rotr32(w[i-15],18) ^ (w[i-15]>>3);
        uint32_t s1 = rotr32(w[i-2],17) ^ rotr32(w[i-2],19) ^ (w[i-2]>>10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3];
    uint32_t e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for (int i=0;i<64;i++) {
        uint32_t S1 = rotr32(e,6) ^ rotr32(e,11) ^ rotr32(e,25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + K256[i] + w[i];
        uint32_t S0 = rotr32(a,2) ^ rotr32(a,13) ^ rotr32(a,22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        h=g; g=f; f=e; e=d + temp1; d=c; c=b; b=a; a=temp1 + temp2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(sha256_ctx* ctx, const uint8_t* data, size_t len) {
    size_t i = 0;
    size_t have = (ctx->bitcount >> 3) & 63;
    ctx->bitcount += (uint64_t)len * 8;
    size_t need = 64 - have;
    if (len >= need) {
        if (have) {
            memcpy(ctx->buffer + have, data, need);
            sha256_transform(ctx, ctx->buffer);
            i += need;
            have = 0;
        }
        for (; i + 63 < len; i += 64) {
            sha256_transform(ctx, data + i);
        }
    }
    if (i < len) {
        memcpy(ctx->buffer + have, data + i, len - i);
    }
}

static void sha256_final(sha256_ctx* ctx, uint8_t out[32]) {
    uint8_t pad[64]; memset(pad, 0, sizeof(pad)); pad[0]=0x80;
    uint8_t lenbuf[8];
    uint64_t bits = ctx->bitcount;
    for (int i=0;i<8;i++) lenbuf[7-i] = (uint8_t)(bits >> (i*8));
    size_t have = (ctx->bitcount >> 3) & 63;
    size_t padlen = (have < 56) ? (56 - have) : (120 - have);
    sha256_update(ctx, pad, padlen);
    sha256_update(ctx, lenbuf, 8);
    for (int i=0;i<8;i++) {
        out[i*4] = (uint8_t)(ctx->state[i] >> 24);
        out[i*4+1] = (uint8_t)(ctx->state[i] >> 16);
        out[i*4+2] = (uint8_t)(ctx->state[i] >> 8);
        out[i*4+3] = (uint8_t)(ctx->state[i]);
    }
    secure_memzero(ctx, sizeof(*ctx));
}

static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t len, uint8_t out[32]) {
    uint8_t k_ipad[64], k_opad[64];
    uint8_t keyhash[32];
    if (keylen > 64) {
        sha256_ctx c; sha256_init(&c); sha256_update(&c, key, keylen); sha256_final(&c, keyhash);
        key = keyhash; keylen = 32;
    }
    memset(k_ipad, 0x36, 64);
    memset(k_opad, 0x5c, 64);
    for (size_t i=0;i<keylen;i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }
    uint8_t ihash[32];
    sha256_ctx ci; sha256_init(&ci); sha256_update(&ci, k_ipad, 64); sha256_update(&ci, data, len); sha256_final(&ci, ihash);
    sha256_ctx co; sha256_init(&co); sha256_update(&co, k_opad, 64); sha256_update(&co, ihash, 32); sha256_final(&co, out);
    secure_memzero(k_ipad, sizeof(k_ipad));
    secure_memzero(k_opad, sizeof(k_opad));
    secure_memzero(keyhash, sizeof(keyhash));
    secure_memzero(ihash, sizeof(ihash));
    secure_memzero(&ci, sizeof(ci));
    secure_memzero(&co, sizeof(co));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen, const uint8_t* salt, size_t saltlen, uint32_t iter, uint8_t* out, size_t dklen) {
    uint32_t block_count = (uint32_t)((dklen + 31) / 32);
    uint8_t* asalt = (uint8_t*)malloc(saltlen + 4);
    if (!asalt) return;
    memcpy(asalt, salt, saltlen);
    size_t pos = 0;
    for (uint32_t i=1; i<=block_count; i++) {
        asalt[saltlen+0] = (uint8_t)(i >> 24);
        asalt[saltlen+1] = (uint8_t)(i >> 16);
        asalt[saltlen+2] = (uint8_t)(i >> 8);
        asalt[saltlen+3] = (uint8_t)(i);
        uint8_t U[32], T[32];
        hmac_sha256(password, passlen, asalt, saltlen + 4, U);
        memcpy(T, U, 32);
        for (uint32_t j=1; j<iter; j++) {
            hmac_sha256(password, passlen, U, 32, U);
            for (int k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t l = (dklen - pos > 32) ? 32 : (dklen - pos);
        memcpy(out + pos, T, l);
        pos += l;
        secure_memzero(U, sizeof(U));
        secure_memzero(T, sizeof(T));
    }
    secure_memzero(asalt, saltlen + 4);
    free(asalt);
}

static int constant_time_eq(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t r = 0;
    for (size_t i=0;i<n;i++) r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}

#define SALT_LEN 16
#define DK_LEN 32
#define PBKDF2_ITER 210000

typedef struct {
    char username[64];
    char email[256];
    uint8_t salt[SALT_LEN];
    uint8_t hash[DK_LEN];
    uint32_t iterations;
    int loggedIn;
    int inUse;
} User;

static User USER_STORE[8]; // small in-memory store

static void random_bytes(uint8_t* dst, size_t len) {
    // Use CSPRNG if available; fallback to /dev/urandom else rand seeded
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(dst, 1, len, f);
        fclose(f);
        return;
    }
    // Fallback (not ideal), seed once
    static int seeded = 0;
    if (!seeded) { seeded = 1; srand((unsigned)time(NULL)); }
    for (size_t i=0;i<len;i++) dst[i] = (uint8_t)(rand() & 0xFF);
}

static int validate_username(const char* username) {
    if (!username) return 0;
    size_t n = strlen(username);
    if (n < 3 || n > 50) return 0;
    for (size_t i=0;i<n;i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c=='_' || c=='-' || c=='.')) return 0;
    }
    return 1;
}

static int validate_email(const char* email) {
    if (!email) return 0;
    size_t n = strlen(email);
    if (n < 6 || n > 254) return 0;
    for (size_t i=0;i<n;i++) if (isspace((unsigned char)email[i])) return 0;
    const char* atp = strchr(email, '@');
    if (!atp) return 0;
    if (strchr(atp+1, '@')) return 0;
    size_t local_len = (size_t)(atp - email);
    const char* domain = atp + 1;
    if (local_len == 0 || strlen(domain) < 3) return 0;
    if (!strchr(domain, '.')) return 0;
    return 1;
}

static int is_strong_password(const char* pw) {
    if (!pw) return 0;
    size_t n = strlen(pw);
    if (n < 12 || n > 128) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    const char* syms = "!@#$%^&*()-_=+[]{};:,.?/\\|`~'\"<>";
    for (size_t i=0;i<n;i++) {
        unsigned char c = (unsigned char)pw[i];
        if (isspace(c)) return 0;
        if (isupper(c)) hasU = 1;
        else if (islower(c)) hasL = 1;
        else if (isdigit(c)) hasD = 1;
        else if (strchr(syms, c)) hasS = 1;
    }
    return hasU && hasL && hasD && hasS;
}

static User* find_user(const char* username) {
    for (size_t i=0;i<8;i++) {
        if (USER_STORE[i].inUse && strcmp(USER_STORE[i].username, username) == 0) return &USER_STORE[i];
    }
    return NULL;
}

static int register_user(const char* username, const char* email, const char* password) {
    if (!validate_username(username) || !validate_email(email)) return 0;
    if (!is_strong_password(password)) return 0;
    if (find_user(username)) return 0;
    size_t idx;
    for (idx=0; idx<8; idx++) if (!USER_STORE[idx].inUse) break;
    if (idx == 8) return 0;
    User* u = &USER_STORE[idx];
    memset(u, 0, sizeof(*u));
    strncpy(u->username, username, sizeof(u->username)-1);
    strncpy(u->email, email, sizeof(u->email)-1);
    random_bytes(u->salt, SALT_LEN);
    u->iterations = PBKDF2_ITER;
    uint8_t dk[DK_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), u->salt, SALT_LEN, u->iterations, dk, DK_LEN);
    memcpy(u->hash, dk, DK_LEN);
    secure_memzero(dk, sizeof(dk));
    u->loggedIn = 0;
    u->inUse = 1;
    return 1;
}

static int verify_password(const User* u, const char* provided) {
    uint8_t dk[DK_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)provided, strlen(provided), u->salt, SALT_LEN, u->iterations, dk, DK_LEN);
    int ok = constant_time_eq(u->hash, dk, DK_LEN);
    secure_memzero(dk, sizeof(dk));
    return ok;
}

static int login_user(const char* username, const char* password) {
    User* u = find_user(username);
    if (!u) return 0;
    int ok = verify_password(u, password);
    if (ok) u->loggedIn = 1;
    return ok;
}

static int change_email(const char* username, const char* old_email, const char* new_email, const char* confirm_password) {
    User* u = find_user(username);
    if (!u) return 0;
    if (!u->loggedIn) return 0;
    if (!validate_email(old_email) || !validate_email(new_email)) return 0;
    if (strcmp(u->email, old_email) != 0) return 0;
    if (strcmp(u->email, new_email) == 0) return 0;
    if (!verify_password(u, confirm_password)) return 0;
    strncpy(u->email, new_email, sizeof(u->email)-1);
    u->email[sizeof(u->email)-1] = '\0';
    return 1;
}

static void generate_strong_random_password(char* out, size_t outlen) {
    if (outlen < 13) { if (outlen>0) out[0]='\0'; return; }
    const char* lowers = "abcdefghijklmnopqrstuvwxyz";
    const char* uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char* digits = "0123456789";
    const char* symbols = "!@#$%^&*()-_=+[]{};:,.?/\\|`~'\"<>";
    char all[256]; snprintf(all, sizeof(all), "%s%s%s%s", lowers, uppers, digits, symbols);
    // Ensure complexity with first 4
    size_t pos = 0;
    out[pos++] = lowers[rand() % 26];
    out[pos++] = uppers[rand() % 26];
    out[pos++] = digits[rand() % 10];
    out[pos++] = symbols[rand() % (int)strlen(symbols)];
    size_t need = 16; if (need < 12) need = 12;
    while (pos < need && pos+1 < outlen) {
        out[pos++] = all[rand() % (int)strlen(all)];
    }
    // Shuffle
    for (size_t i = pos - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        char tmp = out[i]; out[i] = out[j]; out[j] = tmp;
    }
    out[pos] = '\0';
}

int main(void) {
    // Seed RNG for demo password generation
    srand((unsigned)time(NULL));

    const char* username = "alice";
    const char* initialEmail = "alice@example.com";
    char strongPassword[64];
    generate_strong_random_password(strongPassword, sizeof(strongPassword));

    printf("Register user: %s\n", register_user(username, initialEmail, strongPassword) ? "true" : "false");

    // Test 1: change without login
    printf("Test1 change without login (expect false): %s\n",
           change_email(username, initialEmail, "alice_new@example.com", strongPassword) ? "true" : "false");

    // Test 2: login wrong password
    printf("Test2 login wrong password (expect false): %s\n",
           login_user(username, "WrongPassw0rd!") ? "true" : "false");

    // Test 3: login correct password
    printf("Test3 login correct password (expect true): %s\n",
           login_user(username, strongPassword) ? "true" : "false");

    // Test 4: change to same email
    printf("Test4 change to same email (expect false): %s\n",
           change_email(username, initialEmail, initialEmail, strongPassword) ? "true" : "false");

    // Test 5: successful email change
    printf("Test5 change email valid (expect true): %s\n",
           change_email(username, initialEmail, "alice_secure@example.net", strongPassword) ? "true" : "false");

    // Zeroize local password
    secure_memzero(strongPassword, sizeof(strongPassword));
    return 0;
}