#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// SHA-256 implementation
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
} SHA256_CTX;

static const uint32_t k256_c[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static uint32_t rotr_c(uint32_t x, uint32_t n){ return (x >> n) | (x << (32u - n)); }
static uint32_t ch_c(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static uint32_t maj_c(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static uint32_t ep0_c(uint32_t x){ return rotr_c(x,2) ^ rotr_c(x,13) ^ rotr_c(x,22); }
static uint32_t ep1_c(uint32_t x){ return rotr_c(x,6) ^ rotr_c(x,11) ^ rotr_c(x,25); }
static uint32_t sig0_c(uint32_t x){ return rotr_c(x,7) ^ rotr_c(x,18) ^ (x >> 3); }
static uint32_t sig1_c(uint32_t x){ return rotr_c(x,17) ^ rotr_c(x,19) ^ (x >> 10); }

static void sha256_transform_c(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i]  = ((uint32_t)data[i*4]) << 24;
        m[i] |= ((uint32_t)data[i*4+1]) << 16;
        m[i] |= ((uint32_t)data[i*4+2]) << 8;
        m[i] |= ((uint32_t)data[i*4+3]);
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sig1_c(m[i-2]) + m[i-7] + sig0_c(m[i-15]) + m[i-16];
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
        uint32_t t1 = h + ep1_c(e) + ch_c(e,f,g) + k256_c[i] + m[i];
        uint32_t t2 = ep0_c(a) + maj_c(a,b,c);
        h = g; g = f; f = e;
        e = d + t1;
        d = c; c = b; b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init_c(SHA256_CTX *ctx) {
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

static void sha256_update_c(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform_c(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final_c(SHA256_CTX *ctx, uint8_t hash[]) {
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform_c(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ull;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform_c(ctx, ctx->data);

    for (i = 0; i < 4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i*8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i*8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i*8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i*8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i*8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i*8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i*8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i*8)) & 0xff);
    }
}

static void sha256_bytes_c(const uint8_t *data, size_t len, uint8_t out[32]) {
    SHA256_CTX ctx;
    sha256_init_c(&ctx);
    sha256_update_c(&ctx, data, len);
    sha256_final_c(&ctx, out);
}

static int ct_eq_str(const char *a, size_t la, const char *b, size_t lb) {
    size_t m = (la > lb) ? la : lb;
    unsigned int diff = (unsigned int)(la ^ lb);
    for (size_t i = 0; i < m; ++i) {
        unsigned char av = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char bv = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(av ^ bv);
    }
    return diff == 0;
}

static int ct_eq_bytes(const uint8_t *a, const uint8_t *b, size_t la, size_t lb) {
    size_t m = (la > lb) ? la : lb;
    unsigned int diff = (unsigned int)(la ^ lb);
    for (size_t i = 0; i < m; ++i) {
        uint8_t av = (i < la) ? a[i] : 0;
        uint8_t bv = (i < lb) ? b[i] : 0;
        diff |= (unsigned int)(av ^ bv);
    }
    return diff == 0;
}

static int hexval_c(char c, uint8_t *v) {
    if (c >= '0' && c <= '9') { *v = (uint8_t)(c - '0'); return 1; }
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') { *v = (uint8_t)(10 + c - 'a'); return 1; }
    return 0;
}

static int url_decode_into(const char *in, char *out, size_t outcap, size_t *outlen) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        char c = in[i];
        if (c == '%') {
            uint8_t hi, lo;
            if (in[i+1] == '\0' || in[i+2] == '\0') return 0;
            if (!hexval_c(in[i+1], &hi) || !hexval_c(in[i+2], &lo)) return 0;
            char dec = (char)((hi << 4) | lo);
            if (oi + 1 >= outcap) return 0;
            out[oi++] = dec;
            i += 2;
        } else if (c == '+') {
            if (oi + 1 >= outcap) return 0;
            out[oi++] = ' ';
        } else {
            if (oi + 1 >= outcap) return 0;
            out[oi++] = c;
        }
    }
    if (oi >= outcap) return 0;
    out[oi] = '\0';
    if (outlen) *outlen = oi;
    return 1;
}

static int is_valid_username_c(const char *u, size_t len) {
    if (len == 0 || len > 64) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = u[i];
        if (!(c == '_' || c == '-' || c == '.' ||
             (c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9'))) return 0;
    }
    return 1;
}

static int get_query_param(const char *query, const char *key, char *out, size_t outcap, size_t *outlen) {
    size_t keylen = strlen(key);
    const char *p = query;
    while (1) {
        const char *amp = strchr(p, '&');
        size_t seglen = amp ? (size_t)(amp - p) : strlen(p);
        if (seglen > 0) {
            const char *eq = memchr(p, '=', seglen);
            size_t klen = eq ? (size_t)(eq - p) : seglen;
            size_t vlen = eq ? (seglen - klen - 1) : 0;
            if (klen == keylen && strncmp(p, key, keylen) == 0) {
                // decode value
                char valbuf[1024];
                size_t outl = 0;
                if (vlen >= sizeof(valbuf)) return 0;
                memcpy(valbuf, eq ? (eq + 1) : "", vlen);
                valbuf[vlen] = '\0';
                if (!url_decode_into(valbuf, out, outcap, &outl)) return 0;
                if (outlen) *outlen = outl;
                return 1;
            }
        }
        if (!amp) break;
        p = amp + 1;
    }
    return 0;
}

const char* check_mod(const char *url) {
    static const char *TRUE_S = "true";
    static const char *FALSE_S = "false";
    static const char *SALT = "u$er-salt::";
    static const char *EXPECTED_USERNAME = "admin";
    static const char *EXPECTED_PASSWORD = "s3cureP@ss";

    if (!url) return FALSE_S;

    // Split path and query
    const char *qmark = strchr(url, '?');
    size_t pathlen = qmark ? (size_t)(qmark - url) : strlen(url);
    if (!(pathlen == strlen("/check_mod") && strncmp(url, "/check_mod", pathlen) == 0)) {
        return FALSE_S;
    }

    const char *query = qmark ? (qmark + 1) : "";

    char username[128];
    char password[512];
    size_t ulen = 0, plen = 0;

    if (!get_query_param(query, "username", username, sizeof(username), &ulen)) return FALSE_S;
    if (!get_query_param(query, "password", password, sizeof(password), &plen)) return FALSE_S;

    if (!is_valid_username_c(username, ulen)) return FALSE_S;
    if (plen == 0 || plen > 256) return FALSE_S;

    if (!ct_eq_str(username, ulen, EXPECTED_USERNAME, strlen(EXPECTED_USERNAME))) return FALSE_S;

    // expected hash = sha256(SALT + EXPECTED_PASSWORD)
    uint8_t expected_hash[32], provided_hash[32];
    char comb1[1024], comb2[1024];
    size_t s1 = strlen(SALT), s2 = strlen(EXPECTED_PASSWORD);
    if (s1 + s2 >= sizeof(comb1)) return FALSE_S;
    memcpy(comb1, SALT, s1); memcpy(comb1 + s1, EXPECTED_PASSWORD, s2);
    sha256_bytes_c((const uint8_t*)comb1, s1 + s2, expected_hash);

    size_t s3 = strlen(SALT);
    if (s3 + plen >= sizeof(comb2)) return FALSE_S;
    memcpy(comb2, SALT, s3); memcpy(comb2 + s3, password, plen);
    sha256_bytes_c((const uint8_t*)comb2, s3 + plen, provided_hash);

    if (!ct_eq_bytes(expected_hash, provided_hash, 32, 32)) return FALSE_S;

    return TRUE_S;
}

int main(void) {
    const char *tests[] = {
        "/check_mod?username=admin&password=s3cureP%40ss",
        "/check_mod?username=admin&password=wrong",
        "/check_mod?username=Admin&password=s3cureP%40ss",
        "/check_mod?username=admin",
        "/other?username=admin&password=s3cureP%40ss"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        printf("%s\n", check_mod(tests[i]));
    }
    return 0;
}