#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// SHA-256 implementation
typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} SHA256_CTX;

static uint32_t rotr32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static uint32_t ch32(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static uint32_t maj32(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t ep0_32(uint32_t x) { return rotr32(x,2) ^ rotr32(x,13) ^ rotr32(x,22); }
static uint32_t ep1_32(uint32_t x) { return rotr32(x,6) ^ rotr32(x,11) ^ rotr32(x,25); }
static uint32_t sig0_32(uint32_t x) { return rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3); }
static uint32_t sig1_32(uint32_t x) { return rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    static const uint32_t K[64] = {
        0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
        0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
        0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
        0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
        0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
        0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
        0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
        0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffacul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
    };
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)data[i*4] << 24) |
               ((uint32_t)data[i*4 + 1] << 16) |
               ((uint32_t)data[i*4 + 2] << 8) |
               ((uint32_t)data[i*4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sig1_32(m[i-2]) + m[i-7] + sig0_32(m[i-15]) + m[i-16];
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
        uint32_t t1 = h + ep1_32(e) + ch32(e, f, g) + K[i] + m[i];
        uint32_t t2 = ep0_32(a) + maj32(a, b, c);
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

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul;
    ctx->state[1] = 0xbb67ae85ul;
    ctx->state[2] = 0x3c6ef372ul;
    ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful;
    ctx->state[5] = 0x9b05688cul;
    ctx->state[6] = 0x1f83d9abul;
    ctx->state[7] = 0x5be0cd19ul;
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

    ctx->bitlen += (uint64_t)ctx->datalen * 8;
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

// HMAC-SHA256
static void hmac_sha256(const uint8_t* key, size_t keylen,
                        const uint8_t* data, size_t datalen,
                        uint8_t out[32]) {
    const size_t block_size = 64;
    uint8_t k0[64];
    uint8_t tmp[32];

    if (keylen > block_size) {
        SHA256_CTX h;
        sha256_init(&h);
        sha256_update(&h, key, keylen);
        sha256_final(&h, tmp);
        memset(k0, 0, sizeof(k0));
        memcpy(k0, tmp, 32);
        memset(tmp, 0, sizeof(tmp));
    } else {
        memset(k0, 0, sizeof(k0));
        memcpy(k0, key, keylen);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i = 0; i < 64; ++i) {
        ipad[i] = k0[i] ^ 0x36;
        opad[i] = k0[i] ^ 0x5c;
    }
    SHA256_CTX hi;
    sha256_init(&hi);
    sha256_update(&hi, ipad, 64);
    sha256_update(&hi, data, datalen);
    sha256_final(&hi, tmp);

    SHA256_CTX ho;
    sha256_init(&ho);
    sha256_update(&ho, opad, 64);
    sha256_update(&ho, tmp, 32);
    sha256_final(&ho, out);

    memset(k0, 0, sizeof(k0));
    memset(ipad, 0, sizeof(ipad));
    memset(opad, 0, sizeof(opad));
    memset(tmp, 0, sizeof(tmp));
}

// PBKDF2-HMAC-SHA256
static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* out, size_t dkLen) {
    uint32_t block_count = (uint32_t)((dkLen + 31) / 32);
    uint8_t* asalt = (uint8_t*)malloc(saltlen + 4);
    if (!asalt) exit(1);
    memcpy(asalt, salt, saltlen);

    uint8_t u[32], t[32];
    size_t pos = 0;
    for (uint32_t i = 1; i <= block_count; ++i) {
        asalt[saltlen]   = (uint8_t)((i >> 24) & 0xff);
        asalt[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
        asalt[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
        asalt[saltlen+3] = (uint8_t)(i & 0xff);

        hmac_sha256(password, passlen, asalt, saltlen + 4, u);
        memcpy(t, u, 32);
        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, passlen, u, 32, u);
            for (int k = 0; k < 32; ++k) t[k] ^= u[k];
        }
        size_t l = (dkLen - pos > 32) ? 32 : (dkLen - pos);
        memcpy(out + pos, t, l);
        pos += l;
    }
    memset(u, 0, sizeof(u));
    memset(t, 0, sizeof(t));
    memset(asalt, 0, saltlen + 4);
    free(asalt);
}

// Secure random
static int secure_random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
    if (BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0) {
        return 0;
    }
    return -1;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return -1;
    size_t r = fread(buf, 1, len, f);
    fclose(f);
    return (r == len) ? 0 : -1;
#endif
}

static char* to_hex(const uint8_t* bytes, size_t len) {
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) exit(1);
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(bytes[i] >> 4) & 0xF];
        out[2*i+1] = hex[bytes[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

char* registerUser(const char* username, const char* password) {
    if (!username || !password) {
        return NULL;
    }
    const uint32_t iterations = 200000;
    uint8_t salt[16];
    if (secure_random_bytes(salt, sizeof(salt)) != 0) {
        return NULL;
    }
    uint8_t dk[32];
    pbkdf2_hmac_sha256((const uint8_t*)password, strlen(password),
                       salt, sizeof(salt), iterations, dk, sizeof(dk));
    char iterbuf[16];
    snprintf(iterbuf, sizeof(iterbuf), "%u", iterations);
    char* salt_hex = to_hex(salt, sizeof(salt));
    char* dk_hex = to_hex(dk, sizeof(dk));

    size_t outlen = strlen(username) + 1 + strlen("PBKDF2$SHA256$") + strlen(iterbuf) + 1 + strlen(salt_hex) + 1 + strlen(dk_hex) + 1;
    char* out = (char*)malloc(outlen);
    if (!out) exit(1);
    snprintf(out, outlen, "%s:%s%s$%s$%s", username, "PBKDF2$SHA256$", iterbuf, salt_hex, dk_hex);

    // wipe sensitive
    memset(dk, 0, sizeof(dk));
    memset(salt, 0, sizeof(salt));
    free(salt_hex);
    free(dk_hex);
    return out;
}

int main(void) {
    const char* users[5] = {"alice", "bob", "charlie", "dana", "eve"};
    const char* pwds[5]  = {"Str0ng!Pass#1", "Another$ecret2", "Pa55_word^3", "Secur3Key&4", "MyP@ssw0rd_5"};
    for (int i = 0; i < 5; ++i) {
        char* res = registerUser(users[i], pwds[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            fprintf(stderr, "Registration failed for %s\n", users[i]);
        }
    }
    return 0;
}