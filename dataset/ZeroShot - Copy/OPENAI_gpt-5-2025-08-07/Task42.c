#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// Minimal SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256 implementation and simple in-memory auth

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static inline uint32_t ROTR32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x) { return ROTR32(x, 2) ^ ROTR32(x, 13) ^ ROTR32(x, 22); }
static inline uint32_t EP1(uint32_t x) { return ROTR32(x, 6) ^ ROTR32(x, 11) ^ ROTR32(x, 25); }
static inline uint32_t SIG0(uint32_t x) { return ROTR32(x, 7) ^ ROTR32(x, 18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x) { return ROTR32(x, 17) ^ ROTR32(x, 19) ^ (x >> 10); }

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

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)data[i * 4] << 24) |
               ((uint32_t)data[i * 4 + 1] << 16) |
               ((uint32_t)data[i * 4 + 2] << 8) |
               ((uint32_t)data[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }
    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + EP1(e) + CH(e, f, g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;

    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;

    uint64_t total_bits = ctx->bitlen + (ctx->datalen * 8);
    for (int j = 7; j >= 0; --j) {
        ctx->data[56 + (7 - j)] = (uint8_t)((total_bits >> (j * 8)) & 0xff);
    }
    sha256_transform(ctx, ctx->data);

    for (int j = 0; j < 8; ++j) {
        hash[j * 4 + 0] = (uint8_t)((ctx->state[j] >> 24) & 0xff);
        hash[j * 4 + 1] = (uint8_t)((ctx->state[j] >> 16) & 0xff);
        hash[j * 4 + 2] = (uint8_t)((ctx->state[j] >> 8) & 0xff);
        hash[j * 4 + 3] = (uint8_t)((ctx->state[j]) & 0xff);
    }
}

static void secure_memzero(void* v, size_t n) {
    volatile uint8_t* p = (volatile uint8_t*)v;
    while (n--) *p++ = 0;
}

static void hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t out[32]) {
    uint8_t kipad[64], kopad[64], keyhash[32];
    const uint8_t* k = key;
    size_t klen = key_len;

    if (klen > 64) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, key_len);
        sha256_final(&tctx, keyhash);
        k = keyhash;
        klen = 32;
    }

    memset(kipad, 0x36, 64);
    memset(kopad, 0x5c, 64);
    for (size_t i = 0; i < klen; ++i) {
        kipad[i] ^= k[i];
        kopad[i] ^= k[i];
    }

    uint8_t inner[32];
    SHA256_CTX ictx;
    sha256_init(&ictx);
    sha256_update(&ictx, kipad, 64);
    sha256_update(&ictx, data, data_len);
    sha256_final(&ictx, inner);

    SHA256_CTX octx;
    sha256_init(&octx);
    sha256_update(&octx, kopad, 64);
    sha256_update(&octx, inner, 32);
    sha256_final(&octx, out);

    secure_memzero(kipad, sizeof(kipad));
    secure_memzero(kopad, sizeof(kopad));
    secure_memzero(keyhash, sizeof(keyhash));
    secure_memzero(inner, sizeof(inner));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t pass_len,
                               const uint8_t* salt, size_t salt_len,
                               uint32_t iterations, uint8_t* out, size_t dkLen) {
    uint32_t block_count = (uint32_t)((dkLen + 31) / 32);
    uint8_t* salt_block = (uint8_t*)malloc(salt_len + 4);
    if (!salt_block) return;
    memcpy(salt_block, salt, salt_len);

    for (uint32_t i = 1; i <= block_count; ++i) {
        salt_block[salt_len + 0] = (uint8_t)((i >> 24) & 0xff);
        salt_block[salt_len + 1] = (uint8_t)((i >> 16) & 0xff);
        salt_block[salt_len + 2] = (uint8_t)((i >> 8) & 0xff);
        salt_block[salt_len + 3] = (uint8_t)(i & 0xff);

        uint8_t u[32];
        uint8_t t[32];
        hmac_sha256(password, pass_len, salt_block, salt_len + 4, u);
        memcpy(t, u, 32);

        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, pass_len, u, 32, u);
            for (int k = 0; k < 32; ++k) t[k] ^= u[k];
        }

        size_t offset = (i - 1) * 32;
        size_t cp = (dkLen - offset) < 32 ? (dkLen - offset) : 32;
        memcpy(out + offset, t, cp);

        secure_memzero(u, sizeof(u));
        secure_memzero(t, sizeof(t));
    }
    secure_memzero(salt_block, salt_len + 4);
    free(salt_block);
}

static int ct_equal(const uint8_t* a, const uint8_t* b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; ++i) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

static int get_random_bytes(uint8_t* buf, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t r = fread(buf, 1, len, f);
    fclose(f);
    return r == len;
}

#define MAX_USERS 100
#define USERNAME_MAX 64
#define SALT_LEN 16
#define DK_LEN 32
#define ITERATIONS 100000

typedef struct {
    int in_use;
    char username[USERNAME_MAX];
    uint8_t salt[SALT_LEN];
    uint8_t hash[DK_LEN];
    uint32_t iterations;
} UserRecord;

typedef struct {
    UserRecord users[MAX_USERS];
} AuthSystem;

static void authsystem_init(AuthSystem* as) {
    memset(as, 0, sizeof(*as));
}

static int valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 50) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 1;
}

static int valid_password(const char* p) {
    if (!p) return 0;
    size_t len = strlen(p);
    if (len < 8) return 0;
    int up=0, lo=0, di=0, sy=0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) up = 1;
        else if (islower(c)) lo = 1;
        else if (isdigit(c)) di = 1;
        else sy = 1;
    }
    return up && lo && di && sy;
}

static UserRecord* find_user(AuthSystem* as, const char* username) {
    for (int i = 0; i < MAX_USERS; ++i) {
        if (as->users[i].in_use && strcmp(as->users[i].username, username) == 0) {
            return &as->users[i];
        }
    }
    return NULL;
}

static int register_user(AuthSystem* as, const char* username, const char* password) {
    if (!valid_username(username) || !valid_password(password)) return 0;
    if (find_user(as, username)) return 0;

    int slot = -1;
    for (int i = 0; i < MAX_USERS; ++i) {
        if (!as->users[i].in_use) { slot = i; break; }
    }
    if (slot < 0) return 0;

    UserRecord* rec = &as->users[slot];
    memset(rec, 0, sizeof(*rec));
    strncpy(rec->username, username, USERNAME_MAX - 1);
    if (!get_random_bytes(rec->salt, SALT_LEN)) return 0;

    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       rec->salt, SALT_LEN, ITERATIONS, rec->hash, DK_LEN);
    rec->iterations = ITERATIONS;
    rec->in_use = 1;
    return 1;
}

static int authenticate_user(AuthSystem* as, const char* username, const char* password) {
    UserRecord* rec = find_user(as, username);
    if (!rec) return 0;
    uint8_t dk[DK_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       rec->salt, SALT_LEN, rec->iterations, dk, DK_LEN);
    int ok = ct_equal(dk, rec->hash, DK_LEN);
    secure_memzero(dk, sizeof(dk));
    return ok;
}

int main(void) {
    AuthSystem as;
    authsystem_init(&as);

    int t1 = register_user(&as, "alice", "S3cure!Passw0rd");
    printf("Test1 register alice strong pwd: %s\n", t1 ? "true" : "false");

    int t2 = authenticate_user(&as, "alice", "S3cure!Passw0rd");
    printf("Test2 auth alice correct: %s\n", t2 ? "true" : "false");

    int t3 = authenticate_user(&as, "alice", "WrongPass!1");
    printf("Test3 auth alice wrong: %s\n", t3 ? "true" : "false");

    int t4 = register_user(&as, "alice", "An0ther!Pass");
    printf("Test4 duplicate register alice: %s\n", t4 ? "true" : "false");

    int t5a = register_user(&as, "bob", "short1!");
    int t5b = register_user(&as, "bob", "G00d!Passw0rd");
    int t5c = authenticate_user(&as, "bob", "G00d!Passw0rd");
    printf("Test5 bob short register (expect false): %s, then good register: %s, auth: %s\n",
           t5a ? "true" : "false", t5b ? "true" : "false", t5c ? "true" : "false");

    return 0;
}