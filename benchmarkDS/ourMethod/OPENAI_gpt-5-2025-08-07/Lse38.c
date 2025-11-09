/*
Simple secure login simulation in C.

- Validates username and password against an in-memory "database".
- Passwords are stored as PBKDF2-HMAC-SHA-256 hashes with per-user random salts.
- Returns redirect path based on authentication result.
- Includes input validation, constant-time comparison, and secure zeroization.
- Five test cases included in main().

Note: In production, load user records from a secure database and never log sensitive data.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

/* ========================= Security Helpers ========================= */

static void secure_bzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) { *p++ = 0; }
}

static int secure_memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    unsigned char r = 0;
    for (size_t i = 0; i < n; i++) {
        r |= (unsigned char)(pa[i] ^ pb[i]);
    }
    return r; /* 0 if equal, non-zero otherwise */
}

static int get_random_bytes(unsigned char *out, size_t len) {
#if defined(_WIN32)
    if (BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0) {
        return 1;
    }
    return 0;
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t got = fread(out, 1, len, f);
    fclose(f);
    if (got != len) return 0;
    return 1;
#endif
}

/* ========================= SHA-256 Implementation ========================= */

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    unsigned char data[64];
    size_t datalen;
} sha256_ctx;

static const uint32_t k_sha256[64] = {
    0x428a2f98U,0x71374491U,0xb5c0fbcfU,0xe9b5dba5U,0x3956c25bU,0x59f111f1U,0x923f82a4U,0xab1c5ed5U,
    0xd807aa98U,0x12835b01U,0x243185beU,0x550c7dc3U,0x72be5d74U,0x80deb1feU,0x9bdc06a7U,0xc19bf174U,
    0xe49b69c1U,0xefbe4786U,0x0fc19dc6U,0x240ca1ccU,0x2de92c6fU,0x4a7484aaU,0x5cb0a9dcU,0x76f988daU,
    0x983e5152U,0xa831c66dU,0xb00327c8U,0xbf597fc7U,0xc6e00bf3U,0xd5a79147U,0x06ca6351U,0x14292967U,
    0x27b70a85U,0x2e1b2138U,0x4d2c6dfcU,0x53380d13U,0x650a7354U,0x766a0abbU,0x81c2c92eU,0x92722c85U,
    0xa2bfe8a1U,0xa81a664bU,0xc24b8b70U,0xc76c51a3U,0xd192e819U,0xd6990624U,0xf40e3585U,0x106aa070U,
    0x19a4c116U,0x1e376c08U,0x2748774cU,0x34b0bcb5U,0x391c0cb3U,0x4ed8aa4aU,0x5b9cca4fU,0x682e6ff3U,
    0x748f82eeU,0x78a5636fU,0x84c87814U,0x8cc70208U,0x90befffaU,0xa4506cebU,0xbef9a3f7U,0xc67178f2U
};

static uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32U - n)); }
static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t bsig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
static uint32_t bsig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
static uint32_t ssig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
static uint32_t ssig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

static void sha256_transform(sha256_ctx *ctx, const unsigned char data[64]) {
    uint32_t m[64];
    uint32_t a,b,c,d,e,f,g,h;

    for (int i = 0; i < 16; i++) {
        m[i] = ((uint32_t)data[i*4] << 24) |
               ((uint32_t)data[i*4 + 1] << 16) |
               ((uint32_t)data[i*4 + 2] << 8) |
               ((uint32_t)data[i*4 + 3]);
    }
    for (int i = 16; i < 64; i++) {
        m[i] = ssig0(m[i-15]) + m[i-7] + ssig1(m[i-2]) + m[i-16];
    }

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + bsig1(e) + ch(e, f, g) + k_sha256[i] + m[i];
        uint32_t t2 = bsig0(a) + maj(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;

    secure_bzero(m, sizeof(m));
}

static void sha256_init(sha256_ctx *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667U;
    ctx->state[1] = 0xbb67ae85U;
    ctx->state[2] = 0x3c6ef372U;
    ctx->state[3] = 0xa54ff53aU;
    ctx->state[4] = 0x510e527fU;
    ctx->state[5] = 0x9b05688cU;
    ctx->state[6] = 0x1f83d9abU;
    ctx->state[7] = 0x5be0cd19U;
}

static void sha256_update(sha256_ctx *ctx, const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(sha256_ctx *ctx, unsigned char hash[32]) {
    size_t i = ctx->datalen;

    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;

    ctx->bitlen += (uint64_t)ctx->datalen * 8ULL;
    ctx->data[63] = (unsigned char)(ctx->bitlen      );
    ctx->data[62] = (unsigned char)(ctx->bitlen >> 8 );
    ctx->data[61] = (unsigned char)(ctx->bitlen >> 16);
    ctx->data[60] = (unsigned char)(ctx->bitlen >> 24);
    ctx->data[59] = (unsigned char)(ctx->bitlen >> 32);
    ctx->data[58] = (unsigned char)(ctx->bitlen >> 40);
    ctx->data[57] = (unsigned char)(ctx->bitlen >> 48);
    ctx->data[56] = (unsigned char)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);

    for (i = 0; i < 4; i++) {
        hash[i    ] = (unsigned char)((ctx->state[0] >> (24 - i*8)) & 0xFF);
        hash[i + 4] = (unsigned char)((ctx->state[1] >> (24 - i*8)) & 0xFF);
        hash[i + 8] = (unsigned char)((ctx->state[2] >> (24 - i*8)) & 0xFF);
        hash[i +12] = (unsigned char)((ctx->state[3] >> (24 - i*8)) & 0xFF);
        hash[i +16] = (unsigned char)((ctx->state[4] >> (24 - i*8)) & 0xFF);
        hash[i +20] = (unsigned char)((ctx->state[5] >> (24 - i*8)) & 0xFF);
        hash[i +24] = (unsigned char)((ctx->state[6] >> (24 - i*8)) & 0xFF);
        hash[i +28] = (unsigned char)((ctx->state[7] >> (24 - i*8)) & 0xFF);
    }
    secure_bzero(ctx, sizeof(*ctx));
}

/* ========================= HMAC-SHA256 ========================= */

static void hmac_sha256(const unsigned char *key, size_t key_len,
                        const unsigned char *data, size_t data_len,
                        unsigned char out[32]) {
    unsigned char k_ipad[64];
    unsigned char k_opad[64];
    unsigned char tk[32];

    if (key_len > 64) {
        sha256_ctx tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, key_len);
        sha256_final(&tctx, tk);
        key = tk;
        key_len = 32;
    }
    memset(k_ipad, 0x36, sizeof(k_ipad));
    memset(k_opad, 0x5c, sizeof(k_opad));
    for (size_t i = 0; i < key_len; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    sha256_ctx ctx;
    unsigned char inner_hash[32];

    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, inner_hash, 32);
    sha256_final(&ctx, out);

    secure_bzero(k_ipad, sizeof(k_ipad));
    secure_bzero(k_opad, sizeof(k_opad));
    secure_bzero(tk, sizeof(tk));
    secure_bzero(inner_hash, sizeof(inner_hash));
}

/* ========================= PBKDF2-HMAC-SHA256 ========================= */

static int pbkdf2_hmac_sha256(const unsigned char *password, size_t password_len,
                              const unsigned char *salt, size_t salt_len,
                              uint32_t iterations,
                              unsigned char *out, size_t out_len) {
    if (out_len == 0) return 0;
    const size_t hlen = 32;
    uint32_t block_count = (uint32_t)((out_len + hlen - 1) / hlen);
    unsigned char U[32];
    unsigned char T[32];
    unsigned char *outp = out;

    for (uint32_t i = 1; i <= block_count; i++) {
        unsigned char be_i[4] = {
            (unsigned char)((i >> 24) & 0xFF),
            (unsigned char)((i >> 16) & 0xFF),
            (unsigned char)((i >> 8) & 0xFF),
            (unsigned char)(i & 0xFF)
        };

        /* U1 = HMAC(P, S || INT(i)) */
        sha256_ctx ctx; (void)ctx;
        unsigned char *salt_block = NULL;
        size_t sb_len = 0;

        /* Allocate temporary buffer for salt || INT(i) */
        if (salt_len > SIZE_MAX - 4) return 0; /* overflow check */
        sb_len = salt_len + 4;
        salt_block = (unsigned char *)malloc(sb_len);
        if (!salt_block) return 0;
        memcpy(salt_block, salt, salt_len);
        memcpy(salt_block + salt_len, be_i, 4);

        hmac_sha256(password, password_len, salt_block, sb_len, U);
        memcpy(T, U, 32);

        for (uint32_t j = 2; j <= iterations; j++) {
            hmac_sha256(password, password_len, U, 32, U);
            for (size_t k = 0; k < 32; k++) {
                T[k] ^= U[k];
            }
        }

        size_t to_copy = (out_len < hlen) ? out_len : hlen;
        memcpy(outp, T, to_copy);
        outp += to_copy;
        out_len -= to_copy;

        secure_bzero(U, sizeof(U));
        secure_bzero(T, sizeof(T));
        secure_bzero(salt_block, sb_len);
        free(salt_block);
    }
    return 1;
}

/* ========================= Input Validation ========================= */

#define USERNAME_MIN_LEN 3u
#define USERNAME_MAX_LEN 32u
#define PASSWORD_MIN_LEN 8u
#define PASSWORD_MAX_LEN 128u

static int is_allowed_username_char(char c) {
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '.' || c == '_' || c == '-') {
        return 1;
    }
    return 0;
}

static int validate_username(const char *username) {
    if (!username) return 0;
    size_t len = strnlen(username, USERNAME_MAX_LEN + 1);
    if (len < USERNAME_MIN_LEN || len > USERNAME_MAX_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!is_allowed_username_char(username[i])) return 0;
    }
    return 1;
}

static int validate_password(const char *password) {
    if (!password) return 0;
    size_t len = strnlen(password, PASSWORD_MAX_LEN + 1);
    if (len < PASSWORD_MIN_LEN || len > PASSWORD_MAX_LEN) return 0;

    /* Enforce at least one of each: upper, lower, digit, special */
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (c < 0x20 || c > 0x7E) return 0; /* printable ASCII only */
        if (c >= 'A' && c <= 'Z') has_upper = 1;
        else if (c >= 'a' && c <= 'z') has_lower = 1;
        else if (c >= '0' && c <= '9') has_digit = 1;
        else has_special = 1;
    }
    if (!has_upper || !has_lower || !has_digit || !has_special) return 0;
    return 1;
}

/* ========================= User Database ========================= */

#define SALT_LEN 16u
#define HASH_LEN 32u
#define PBKDF2_ITERS 210000u

typedef struct {
    char username[USERNAME_MAX_LEN + 1];
    unsigned char salt[SALT_LEN];
    uint32_t iterations;
    unsigned char pwd_hash[HASH_LEN];
} UserRecord;

typedef struct {
    UserRecord *records;
    size_t count;
} UserDB;

static int userdb_init(UserDB *db, size_t capacity) {
    if (!db) return 0;
    db->records = (UserRecord *)calloc(capacity, sizeof(UserRecord));
    if (!db->records) return 0;
    db->count = 0;
    return 1;
}

static void userdb_free(UserDB *db) {
    if (!db) return;
    if (db->records) {
        /* Clear sensitive data before free */
        for (size_t i = 0; i < db->count; i++) {
            secure_bzero(db->records[i].pwd_hash, HASH_LEN);
            secure_bzero(db->records[i].salt, SALT_LEN);
            secure_bzero(db->records[i].username, sizeof(db->records[i].username));
        }
        free(db->records);
        db->records = NULL;
    }
    db->count = 0;
}

static int userdb_add(UserDB *db, const char *username, const char *password) {
    if (!db || !db->records) return 0;
    if (!validate_username(username)) return 0;
    if (!validate_password(password)) return 0;

    /* Check duplicate username */
    for (size_t i = 0; i < db->count; i++) {
        if (strncmp(db->records[i].username, username, USERNAME_MAX_LEN) == 0) {
            return 0; /* already exists */
        }
    }

    UserRecord *rec = &db->records[db->count];
    size_t ulen = strnlen(username, USERNAME_MAX_LEN);
    memcpy(rec->username, username, ulen);
    rec->username[ulen] = '\0';
    rec->iterations = PBKDF2_ITERS;

    if (!get_random_bytes(rec->salt, SALT_LEN)) {
        return 0;
    }

    if (!pbkdf2_hmac_sha256((const unsigned char *)password, strnlen(password, PASSWORD_MAX_LEN + 1),
                            rec->salt, SALT_LEN, rec->iterations,
                            rec->pwd_hash, HASH_LEN)) {
        return 0;
    }

    db->count++;
    return 1;
}

static const UserRecord* userdb_find(const UserDB *db, const char *username) {
    if (!db || !username) return NULL;
    if (!validate_username(username)) return NULL;
    for (size_t i = 0; i < db->count; i++) {
        if (strncmp(db->records[i].username, username, USERNAME_MAX_LEN) == 0) {
            return &db->records[i];
        }
    }
    return NULL;
}

/* ========================= Authentication Logic ========================= */

static int validate_credentials(const UserDB *db, const char *username, const char *password) {
    if (!db || !username || !password) return 0;
    /* Input validation */
    if (!validate_username(username)) return 0;
    /* For login, allow any length >= 1 password attempt, but cap to max; we won't enforce complexity here to avoid leaking info */
    size_t pass_len = strnlen(password, PASSWORD_MAX_LEN + 1);
    if (pass_len == 0 || pass_len > PASSWORD_MAX_LEN) return 0;
    /* Optional: restrict to printable ASCII to avoid encoding issues */
    for (size_t i = 0; i < pass_len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (c < 0x20 || c > 0x7E) return 0;
    }

    const UserRecord *rec = userdb_find(db, username);
    if (!rec) {
        /* Do a dummy hash computation to equalize timing */
        unsigned char dummy_salt[SALT_LEN];
        unsigned char dummy_out[HASH_LEN];
        memset(dummy_salt, 0xAA, sizeof(dummy_salt));
        (void)pbkdf2_hmac_sha256((const unsigned char *)password, pass_len,
                                 dummy_salt, sizeof(dummy_salt), PBKDF2_ITERS,
                                 dummy_out, sizeof(dummy_out));
        secure_bzero(dummy_out, sizeof(dummy_out));
        return 0;
    }

    unsigned char candidate[HASH_LEN];
    if (!pbkdf2_hmac_sha256((const unsigned char *)password, pass_len,
                            rec->salt, SALT_LEN, rec->iterations,
                            candidate, HASH_LEN)) {
        secure_bzero(candidate, sizeof(candidate));
        return 0;
    }

    int cmp = secure_memcmp(rec->pwd_hash, candidate, HASH_LEN);
    secure_bzero(candidate, sizeof(candidate));
    return cmp == 0 ? 1 : 0;
}

static const char* handle_login(const UserDB *db, const char *username, const char *password) {
    int ok = validate_credentials(db, username, password);
    if (ok) {
        return "/dashboard";
    }
    return "/login";
}

/* ========================= Test Harness ========================= */

static void print_hex(const unsigned char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        unsigned char hi = (unsigned char)((buf[i] >> 4) & 0xF);
        unsigned char lo = (unsigned char)(buf[i] & 0xF);
        putchar("0123456789abcdef"[hi]);
        putchar("0123456789abcdef"[lo]);
    }
}

int main(void) {
    /* Initialize user database with capacity for 3 users */
    UserDB db;
    if (!userdb_init(&db, 3)) {
        fprintf(stderr, "Initialization failed.\n");
        return 1;
    }

    /* Register three users with strong passwords (for demonstration) */
    const char *u1 = "alice";
    const char *p1 = "S3cur3!Pass1";
    const char *u2 = "bob";
    const char *p2 = "Another$Pass2";
    const char *u3 = "carol";
    const char *p3 = "Y3t@Pass3";

    if (!userdb_add(&db, u1, p1)) { fprintf(stderr, "Failed to add user 1.\n"); userdb_free(&db); return 1; }
    if (!userdb_add(&db, u2, p2)) { fprintf(stderr, "Failed to add user 2.\n"); userdb_free(&db); return 1; }
    if (!userdb_add(&db, u3, p3)) { fprintf(stderr, "Failed to add user 3.\n"); userdb_free(&db); return 1; }

    /* Optional: show that hashes and salts exist (do not print in production) */
    /*
    for (size_t i = 0; i < db.count; i++) {
        printf("User %zu: %s\n  salt: ", i+1, db.records[i].username);
        print_hex(db.records[i].salt, SALT_LEN);
        printf("\n  hash: ");
        print_hex(db.records[i].pwd_hash, HASH_LEN);
        printf("\n");
    }
    */

    /* Test cases (do not print sensitive inputs) */
    struct {
        const char *desc;
        const char *username;
        const char *password;
        const char *expected_redirect;
    } tests[5] = {
        { "1) Correct credentials", "alice", "S3cur3!Pass1", "/dashboard" },
        { "2) Wrong password", "alice", "Wrong!Pass1", "/login" },
        { "3) Nonexistent user", "dave", "S0me$Pass!", "/login" },
        { "4) Invalid username format", "eve;drop", "Good@Pass9", "/login" },
        { "5) Too short password", "bob", "short1!", "/login" }
    };

    for (size_t i = 0; i < 5; i++) {
        const char *redirect = handle_login(&db, tests[i].username, tests[i].password);
        printf("%s -> Redirect: %s\n", tests[i].desc, redirect);
        if (strcmp(redirect, tests[i].expected_redirect) != 0) {
            fprintf(stderr, "Test %zu failed (expected %s)\n", i + 1, tests[i].expected_redirect);
        }
    }

    userdb_free(&db);
    return 0;
}