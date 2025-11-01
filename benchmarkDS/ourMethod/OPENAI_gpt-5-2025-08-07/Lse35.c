#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// Constants
#define SALT_LEN 16
#define HASH_LEN 32  // 256-bit
#define ITERATIONS 210000u
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 128

static const char* LOGIN_HTML =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head><meta charset=\"UTF-8\"><title>Login</title></head>\n"
"<body>\n"
"  <h1>Login</h1>\n"
"  <form action=\"/login\" method=\"POST\">\n"
"    <label>Username: <input type=\"text\" name=\"username\" maxlength=\"50\" required></label><br>\n"
"    <label>Password: <input type=\"password\" name=\"password\" maxlength=\"128\" required></label><br>\n"
"    <button type=\"submit\">Login</button>\n"
"  </form>\n"
"</body>\n"
"</html>\n";

// Secure zero
static void secure_zero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

// Random bytes via /dev/urandom
static int random_bytes(uint8_t* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t r = fread(out, 1, len, f);
    fclose(f);
    return r == len;
}

// SHA-256 implementation
typedef struct {
    uint64_t bitlen;
    uint32_t state[8];
    uint8_t data[64];
    size_t datalen;
} sha256_ctx;

static uint32_t sha_rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static uint32_t sha_ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static uint32_t sha_maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t sha_ep0(uint32_t x) { return sha_rotr(x, 2) ^ sha_rotr(x, 13) ^ sha_rotr(x, 22); }
static uint32_t sha_ep1(uint32_t x) { return sha_rotr(x, 6) ^ sha_rotr(x, 11) ^ sha_rotr(x, 25); }
static uint32_t sha_sig0(uint32_t x) { return sha_rotr(x, 7) ^ sha_rotr(x, 18) ^ (x >> 3); }
static uint32_t sha_sig1(uint32_t x) { return sha_rotr(x, 17) ^ sha_rotr(x, 19) ^ (x >> 10); }

static void sha256_init(sha256_ctx* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

static void sha256_transform(sha256_ctx* ctx, const uint8_t data[]) {
    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)data[i * 4] << 24) |
               ((uint32_t)data[i * 4 + 1] << 16) |
               ((uint32_t)data[i * 4 + 2] << 8) |
               ((uint32_t)data[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sha_sig1(m[i - 2]) + m[i - 7] + sha_sig0(m[i - 15]) + m[i - 16];
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
        uint32_t t1 = h + sha_ep1(e) + sha_ch(e, f, g) + k[i] + m[i];
        uint32_t t2 = sha_ep0(a) + sha_maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
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

static void sha256_update(sha256_ctx* ctx, const uint8_t* in, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = in[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(sha256_ctx* ctx, uint8_t out[32]) {
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (int j = 0; j < 8; ++j) {
        out[j * 4] = (uint8_t)((ctx->state[j] >> 24) & 0xff);
        out[j * 4 + 1] = (uint8_t)((ctx->state[j] >> 16) & 0xff);
        out[j * 4 + 2] = (uint8_t)((ctx->state[j] >> 8) & 0xff);
        out[j * 4 + 3] = (uint8_t)(ctx->state[j] & 0xff);
    }
}

static void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, out);
}

// HMAC-SHA256
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]) {
    const size_t block = 64;
    uint8_t k0[64];
    memset(k0, 0, sizeof(k0));
    if (keylen > block) {
        uint8_t kh[32];
        sha256(key, keylen, kh);
        memcpy(k0, kh, 32);
        secure_zero(kh, sizeof(kh));
    } else {
        memcpy(k0, key, keylen);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i = 0; i < 64; ++i) {
        ipad[i] = k0[i] ^ 0x36;
        opad[i] = k0[i] ^ 0x5c;
    }
    // inner
    uint8_t* inner = (uint8_t*)malloc(64 + datalen);
    if (!inner) exit(1);
    memcpy(inner, ipad, 64);
    memcpy(inner + 64, data, datalen);
    uint8_t ihash[32];
    sha256(inner, 64 + datalen, ihash);
    // outer
    uint8_t outer[64 + 32];
    memcpy(outer, opad, 64);
    memcpy(outer + 64, ihash, 32);
    sha256(outer, 96, out);

    secure_zero(k0, sizeof(k0));
    secure_zero(ipad, sizeof(ipad));
    secure_zero(opad, sizeof(opad));
    secure_zero(ihash, sizeof(ihash));
    secure_zero(inner, 64 + datalen);
    free(inner);
    secure_zero(outer, sizeof(outer));
}

// PBKDF2-HMAC-SHA256
static void pbkdf2_hmac_sha256(const uint8_t* pwd, size_t pwdlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iters, uint8_t* out, size_t outlen) {
    size_t blocks = (outlen + 31) / 32;
    uint8_t* asalt = (uint8_t*)malloc(saltlen + 4);
    if (!asalt) exit(1);
    memcpy(asalt, salt, saltlen);

    for (size_t i = 1; i <= blocks; ++i) {
        asalt[saltlen + 0] = (uint8_t)((i >> 24) & 0xff);
        asalt[saltlen + 1] = (uint8_t)((i >> 16) & 0xff);
        asalt[saltlen + 2] = (uint8_t)((i >> 8) & 0xff);
        asalt[saltlen + 3] = (uint8_t)(i & 0xff);

        uint8_t u[32];
        uint8_t t[32];
        hmac_sha256(pwd, pwdlen, asalt, saltlen + 4, u);
        memcpy(t, u, 32);
        for (uint32_t j = 1; j < iters; ++j) {
            hmac_sha256(pwd, pwdlen, u, 32, u);
            for (size_t k = 0; k < 32; ++k) t[k] ^= u[k];
        }

        size_t offset = (i - 1) * 32;
        size_t cp = (outlen - offset) > 32 ? 32 : (outlen - offset);
        memcpy(out + offset, t, cp);

        secure_zero(u, sizeof(u));
        secure_zero(t, sizeof(t));
    }
    secure_zero(asalt, saltlen + 4);
    free(asalt);
}

typedef struct {
    uint8_t* salt;
    uint8_t* hash;
} user_record;

typedef struct {
    char* username;
    user_record rec;
} user_entry;

typedef struct {
    user_entry* entries;
    size_t count;
    size_t cap;
} user_db;

static void db_init(user_db* db) {
    db->entries = NULL;
    db->count = 0;
    db->cap = 0;
}

static void db_free(user_db* db) {
    if (!db) return;
    for (size_t i = 0; i < db->count; ++i) {
        free(db->entries[i].username);
        if (db->entries[i].rec.salt) {
            secure_zero(db->entries[i].rec.salt, SALT_LEN);
            free(db->entries[i].rec.salt);
        }
        if (db->entries[i].rec.hash) {
            secure_zero(db->entries[i].rec.hash, HASH_LEN);
            free(db->entries[i].rec.hash);
        }
    }
    free(db->entries);
    db->entries = NULL;
    db->count = db->cap = 0;
}

static user_record* db_get(user_db* db, const char* username) {
    for (size_t i = 0; i < db->count; ++i) {
        if (strcmp(db->entries[i].username, username) == 0) return &db->entries[i].rec;
    }
    return NULL;
}

static void db_add(user_db* db, const char* username, const uint8_t* salt, const uint8_t* hash) {
    if (db->count == db->cap) {
        size_t ncap = db->cap ? db->cap * 2 : 8;
        user_entry* nentries = (user_entry*)realloc(db->entries, ncap * sizeof(user_entry));
        if (!nentries) exit(1);
        db->entries = nentries;
        db->cap = ncap;
    }
    db->entries[db->count].username = strdup(username);
    db->entries[db->count].rec.salt = (uint8_t*)malloc(SALT_LEN);
    db->entries[db->count].rec.hash = (uint8_t*)malloc(HASH_LEN);
    if (!db->entries[db->count].username || !db->entries[db->count].rec.salt || !db->entries[db->count].rec.hash) exit(1);
    memcpy(db->entries[db->count].rec.salt, salt, SALT_LEN);
    memcpy(db->entries[db->count].rec.hash, hash, HASH_LEN);
    db->count++;
}

static bool valid_username(const char* u) {
    if (!u) return false;
    size_t len = strlen(u);
    if (len < 3 || len > MAX_USERNAME_LEN) return false;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}

static bool strong_password(const char* p) {
    if (!p) return false;
    size_t len = strlen(p);
    if (len < 12 || len > MAX_PASSWORD_LEN) return false;
    bool up = false, lo = false, di = false, sp = false;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)p[i];
        if (iscntrl(c)) return false;
        if (isupper(c)) up = true;
        else if (islower(c)) lo = true;
        else if (isdigit(c)) di = true;
        else sp = true;
    }
    return up && lo && di && sp;
}

static void create_user(user_db* db, const char* username, const char* password) {
    if (!valid_username(username)) { fprintf(stderr, "Invalid username\n"); exit(1); }
    if (!strong_password(password)) { fprintf(stderr, "Weak password\n"); exit(1); }
    if (db_get(db, username) != NULL) { fprintf(stderr, "User exists\n"); exit(1); }

    uint8_t salt[SALT_LEN];
    if (!random_bytes(salt, sizeof(salt))) { fprintf(stderr, "RNG failure\n"); exit(1); }

    uint8_t hash[HASH_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, sizeof(salt), ITERATIONS, hash, sizeof(hash));

    db_add(db, username, salt, hash);
    secure_zero(hash, sizeof(hash));
}

static bool ct_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t r = 0;
    for (size_t i = 0; i < n; ++i) r |= (a[i] ^ b[i]);
    return r == 0;
}

static const char* login(user_db* db, const char* username, const char* password) {
    if (!valid_username(username)) return "login.html";
    if (!password) return "login.html";
    size_t plen = strlen(password);
    if (plen < 1 || plen > MAX_PASSWORD_LEN) return "login.html";

    user_record* rec = db_get(db, username);
    if (!rec) return "login.html";

    uint8_t comp[HASH_LEN];
    pbkdf2_hmac_sha256((const uint8_t*)password, plen, rec->salt, SALT_LEN, ITERATIONS, comp, HASH_LEN);
    bool ok = ct_equal(comp, rec->hash, HASH_LEN);
    secure_zero(comp, sizeof(comp));
    return ok ? "home.html" : "login.html";
}

static const char* get_login_html(void) {
    return LOGIN_HTML;
}

int main(void) {
    printf("%s\n", get_login_html());

    user_db db;
    db_init(&db);

    create_user(&db, "user1", "StrongPassw0rd!");
    create_user(&db, "user2", "Compl3x!Pass#2");
    create_user(&db, "user3", "An0ther$trongOne");

    const char* results[5];
    results[0] = login(&db, "user1", "StrongPassw0rd!"); // home
    results[1] = login(&db, "user2", "wrongPassword");   // login
    results[2] = login(&db, "nosuchuser", "Anything1!"); // login
    results[3] = login(&db, "bad user", "Compl3x!Pass#2"); // invalid username -> login
    char longPwd[130];
    memset(longPwd, 'x', 129);
    longPwd[129] = '\0';
    results[4] = login(&db, "user3", longPwd); // too long -> login

    for (int i = 0; i < 5; ++i) {
        printf("Test %d: %s\n", i + 1, results[i]);
    }

    db_free(&db);
    return 0;
}