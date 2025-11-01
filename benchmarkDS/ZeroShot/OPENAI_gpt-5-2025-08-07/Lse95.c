#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// Simple in-memory "DB"
typedef struct {
    char* username;
    char* stored; // hex(hash) : hex(salt)
} kv_pair;

typedef struct {
    kv_pair* items;
    size_t len;
    size_t cap;
} map_t;

static void map_init(map_t* m) { m->items = NULL; m->len = 0; m->cap = 0; }
static void map_free(map_t* m) {
    for (size_t i = 0; i < m->len; ++i) { free(m->items[i].username); free(m->items[i].stored); }
    free(m->items); m->items = NULL; m->len = m->cap = 0;
}
static const char* map_get(map_t* m, const char* key) {
    for (size_t i = 0; i < m->len; ++i) if (strcmp(m->items[i].username, key) == 0) return m->items[i].stored;
    return NULL;
}
static int map_set(map_t* m, const char* key, const char* val) {
    for (size_t i = 0; i < m->len; ++i) {
        if (strcmp(m->items[i].username, key) == 0) {
            free(m->items[i].stored);
            m->items[i].stored = strdup(val);
            return 1;
        }
    }
    if (m->len == m->cap) {
        size_t ncap = m->cap ? m->cap * 2 : 8;
        kv_pair* nitems = (kv_pair*)realloc(m->items, ncap * sizeof(kv_pair));
        if (!nitems) return 0;
        m->items = nitems;
        m->cap = ncap;
    }
    m->items[m->len].username = strdup(key);
    m->items[m->len].stored = strdup(val);
    if (!m->items[m->len].username || !m->items[m->len].stored) return 0;
    m->len++;
    return 1;
}

static map_t DB;

static int valid_username(const char* u) {
    size_t n = strlen(u);
    if (n < 3 || n > 32) return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_')) return 0;
    }
    return 1;
}
static int valid_password(const char* p) {
    return p && strlen(p) >= 8;
}

static int const_time_eq(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t diff = 0;
    for (size_t i = 0; i < n; ++i) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
static char* to_hex(const uint8_t* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hex[data[i] >> 4];
        out[2*i+1] = hex[data[i] & 0x0F];
    }
    out[len*2] = '\0';
    return out;
}
static int from_hex(const char* s, uint8_t* out, size_t outlen) {
    size_t n = strlen(s);
    if (n != outlen * 2) return 0;
    for (size_t i = 0; i < outlen; ++i) {
        int hi = hexval(s[2*i]);
        int lo = hexval(s[2*i+1]);
        if (hi < 0 || lo < 0) return 0;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return 1;
}

static int get_random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
    NTSTATUS st = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return st >= 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t got = 0;
    while (got < len) {
        ssize_t r = read(fd, buf + got, len - got);
        if (r <= 0) { close(fd); return 0; }
        got += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

/*** SHA-256 implementation ***/
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static uint32_t ror(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t BSIG0(uint32_t x) { return ror(x,2) ^ ror(x,13) ^ ror(x,22); }
static uint32_t BSIG1(uint32_t x) { return ror(x,6) ^ ror(x,11) ^ ror(x,25); }
static uint32_t SSIG0(uint32_t x) { return ror(x,7) ^ ror(x,18) ^ (x >> 3); }
static uint32_t SSIG1(uint32_t x) { return ror(x,17) ^ ror(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    static const uint32_t K[64] = {
        0x428a2f98UL,0x71374491UL,0xb5c0fbcfUL,0xe9b5dba5UL,0x3956c25bUL,0x59f111f1UL,0x923f82a4UL,0xab1c5ed5UL,
        0xd807aa98UL,0x12835b01UL,0x243185beUL,0x550c7dc3UL,0x72be5d74UL,0x80deb1feUL,0x9bdc06a7UL,0xc19bf174UL,
        0xe49b69c1UL,0xefbe4786UL,0x0fc19dc6UL,0x240ca1ccUL,0x2de92c6fUL,0x4a7484aaUL,0x5cb0a9dcUL,0x76f988daUL,
        0x983e5152UL,0xa831c66dUL,0xb00327c8UL,0xbf597fc7UL,0xc6e00bf3UL,0xd5a79147UL,0x06ca6351UL,0x14292967UL,
        0x27b70a85UL,0x2e1b2138UL,0x4d2c6dfcUL,0x53380d13UL,0x650a7354UL,0x766a0abbUL,0x81c2c92eUL,0x92722c85UL,
        0xa2bfe8a1UL,0xa81a664bUL,0xc24b8b70UL,0xc76c51a3UL,0xd192e819UL,0xd6990624UL,0xf40e3585UL,0x106aa070UL,
        0x19a4c116UL,0x1e376c08UL,0x2748774cUL,0x34b0bcb5UL,0x391c0cb3UL,0x4ed8aa4aUL,0x5b9cca4fUL,0x682e6ff3UL,
        0x748f82eeUL,0x78a5636fUL,0x84c87814UL,0x8cc70208UL,0x90befffaUL,0xa4506cebUL,0xbef9a3f7UL,0xc67178f2UL
    };
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)data[i*4] << 24) | ((uint32_t)data[i*4+1] << 16) | ((uint32_t)data[i*4+2] << 8) | ((uint32_t)data[i*4+3]);
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = SSIG1(m[i-2]) + m[i-7] + SSIG0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + BSIG1(e) + Ch(e,f,g) + K[i] + m[i];
        uint32_t t2 = BSIG0(a) + Maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667UL; ctx->state[1]=0xbb67ae85UL; ctx->state[2]=0x3c6ef372UL; ctx->state[3]=0xa54ff53aUL;
    ctx->state[4]=0x510e527fUL; ctx->state[5]=0x9b05688cUL; ctx->state[6]=0x1f83d9abUL; ctx->state[7]=0x5be0cd19UL;
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
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i * 8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i * 8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i * 8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i * 8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i * 8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i * 8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i * 8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i * 8)) & 0xff);
    }
}

static void hmac_sha256(const uint8_t* key, size_t keylen,
                        const uint8_t* data, size_t datalen,
                        uint8_t out[32]) {
    uint8_t kopad[64], kipad[64];
    uint8_t tk[32];
    if (keylen > 64) {
        SHA256_CTX t;
        sha256_init(&t);
        sha256_update(&t, key, keylen);
        sha256_final(&t, tk);
        key = tk;
        keylen = 32;
    }
    memset(kipad, 0, 64);
    memset(kopad, 0, 64);
    memcpy(kipad, key, keylen);
    memcpy(kopad, key, keylen);
    for (int i = 0; i < 64; ++i) {
        kipad[i] ^= 0x36;
        kopad[i] ^= 0x5c;
    }
    SHA256_CTX ictx;
    sha256_init(&ictx);
    sha256_update(&ictx, kipad, 64);
    sha256_update(&ictx, data, datalen);
    sha256_final(&ictx, tk);
    SHA256_CTX octx;
    sha256_init(&octx);
    sha256_update(&octx, kopad, 64);
    sha256_update(&octx, tk, 32);
    sha256_final(&octx, out);
    memset(kipad, 0, 64);
    memset(kopad, 0, 64);
    memset(tk, 0, 32);
}

static int pbkdf2_hmac_sha256(const uint8_t* pass, size_t passlen,
                              const uint8_t* salt, size_t saltlen,
                              uint32_t iterations,
                              uint8_t* dk, size_t dklen) {
    const size_t hlen = 32;
    size_t l = (dklen + hlen - 1) / hlen;
    size_t r = dklen - (l - 1) * hlen;
    uint8_t* U = (uint8_t*)malloc(hlen);
    uint8_t* T = (uint8_t*)malloc(hlen);
    uint8_t* saltBlock = (uint8_t*)malloc(saltlen + 4);
    if (!U || !T || !saltBlock) { free(U); free(T); free(saltBlock); return 0; }
    memcpy(saltBlock, salt, saltlen);
    for (size_t i = 1; i <= l; ++i) {
        saltBlock[saltlen]   = (uint8_t)((i >> 24) & 0xff);
        saltBlock[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
        saltBlock[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
        saltBlock[saltlen+3] = (uint8_t)(i & 0xff);
        hmac_sha256(pass, passlen, saltBlock, saltlen + 4, U);
        memcpy(T, U, hlen);
        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(pass, passlen, U, hlen, U);
            for (size_t k = 0; k < hlen; ++k) T[k] ^= U[k];
        }
        size_t toCopy = (i == l) ? r : hlen;
        memcpy(dk + (i - 1) * hlen, T, toCopy);
    }
    free(U); free(T); free(saltBlock);
    return 1;
}

#define SALT_LEN 16
#define KEY_LEN 32
#define ITERATIONS 100000u

// API functions
int Lse95_register_user(const char* username, const char* password) {
    if (!username || !password) return 0;
    if (!valid_username(username) || !valid_password(password)) return 0;
    if (map_get(&DB, username) != NULL) return 0;
    uint8_t salt[SALT_LEN];
    if (!get_random_bytes(salt, SALT_LEN)) return 0;
    uint8_t dk[KEY_LEN];
    if (!pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, SALT_LEN, ITERATIONS, dk, KEY_LEN)) return 0;
    char* hexhash = to_hex(dk, KEY_LEN);
    char* hexsalt = to_hex(salt, SALT_LEN);
    if (!hexhash || !hexsalt) { free(hexhash); free(hexsalt); return 0; }
    size_t stored_len = strlen(hexhash) + 1 + strlen(hexsalt) + 1;
    char* stored = (char*)malloc(stored_len);
    if (!stored) { free(hexhash); free(hexsalt); return 0; }
    snprintf(stored, stored_len, "%s:%s", hexhash, hexsalt);
    int ok = map_set(&DB, username, stored);
    free(hexhash); free(hexsalt); free(stored);
    return ok;
}

int Lse95_login_user(const char* username, const char* password) {
    if (!username || !password) return 0;
    const char* stored = map_get(&DB, username);
    if (!stored) return 0;
    const char* colon = strchr(stored, ':');
    if (!colon) return 0;
    size_t hlen = (size_t)(colon - stored);
    size_t slen = strlen(colon + 1);
    if (hlen != KEY_LEN * 2 || slen != SALT_LEN * 2) return 0;
    uint8_t expected[KEY_LEN], salt[SALT_LEN], dk[KEY_LEN];
    if (!from_hex(stored, expected, KEY_LEN)) return 0;
    if (!from_hex(colon + 1, salt, SALT_LEN)) return 0;
    if (!pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password), salt, SALT_LEN, ITERATIONS, dk, KEY_LEN)) return 0;
    return const_time_eq(expected, dk, KEY_LEN);
}

int main(void) {
    map_init(&DB);
    printf("Register alice: %s\n", Lse95_register_user("alice", "Str0ngP@ss!") ? "true" : "false");
    printf("Login alice (correct): %s\n", Lse95_login_user("alice", "Str0ngP@ss!") ? "true" : "false");
    printf("Register bob: %s\n", Lse95_register_user("bob", "anotherS3cure") ? "true" : "false");
    printf("Login bob (wrong): %s\n", Lse95_login_user("bob", "wrongpass") ? "true" : "false");
    printf("Register alice again (duplicate): %s\n", Lse95_register_user("alice", "NewPassw0rd!") ? "true" : "false");
    printf("Register charlie_01 (too short): %s\n", Lse95_register_user("charlie_01", "short") ? "true" : "false");
    printf("Register dave: %s\n", Lse95_register_user("dave", "CorrectHorseBatteryStaple1!") ? "true" : "false");
    printf("Login dave (correct): %s\n", Lse95_login_user("dave", "CorrectHorseBatteryStaple1!") ? "true" : "false");
    printf("Login unknown user: %s\n", Lse95_login_user("eve", "SomePass123") ? "true" : "false");
    map_free(&DB);
    return 0;
}