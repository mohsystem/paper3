/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Implement PBKDF2-HMAC-SHA256 to compute a salted password hash.
 * 2) Security requirements: Strong KDF with sufficient iterations, safe memory use.
 * 3) Secure coding generation: Implement SHA-256, HMAC, PBKDF2 correctly with bounds checks.
 * 4) Code review: Big-endian handling, padding, no overflows, fixed sizes.
 * 5) Secure code output: 200000 iterations, 32-byte output, hex encoding.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

static inline uint32_t ROTR32(uint32_t x, uint32_t n){ return (x >> n) | (x << (32u - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR32(x,2) ^ ROTR32(x,13) ^ ROTR32(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR32(x,6) ^ ROTR32(x,11) ^ ROTR32(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR32(x,7) ^ ROTR32(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR32(x,17) ^ ROTR32(x,19) ^ (x >> 10); }

static const uint32_t K256_C[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]){
    uint32_t a,b,c,d,e,f,g,h,i,t1,t2,m[64];

    for (i = 0; i < 16; ++i) {
        m[i] = ((uint32_t)data[i*4] << 24) |
               ((uint32_t)data[i*4 + 1] << 16) |
               ((uint32_t)data[i*4 + 2] << 8) |
               ((uint32_t)data[i*4 + 3]);
    }
    for (; i < 64; ++i) {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + K256_C[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
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

static void sha256_init(SHA256_CTX* ctx){
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

static void sha256_update(SHA256_CTX* ctx, const uint8_t data[], size_t len){
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    uint32_t i = ctx->datalen;

    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    ctx->bitlen += (uint64_t)ctx->datalen * 8ull;

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
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i * 8)) & 0xFF);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i * 8)) & 0xFF);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i * 8)) & 0xFF);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i * 8)) & 0xFF);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i * 8)) & 0xFF);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i * 8)) & 0xFF);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i * 8)) & 0xFF);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i * 8)) & 0xFF);
    }

    memset(ctx->data, 0, sizeof(ctx->data));
    ctx->datalen = 0;
    ctx->bitlen = 0;
    memset(ctx->state, 0, sizeof(ctx->state));
}

static void hmac_sha256(const uint8_t* key, size_t keylen,
                        const uint8_t* data, size_t datalen,
                        uint8_t out[32]){
    const size_t blocksize = 64;
    uint8_t kipad[64];
    uint8_t kopad[64];
    uint8_t khash[32];
    uint8_t tk[32];
    const uint8_t* kptr = key;
    size_t klen = keylen;

    if (klen > blocksize) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, kptr, klen);
        sha256_final(&tctx, tk);
        kptr = tk;
        klen = 32;
    }

    memset(kipad, 0, blocksize);
    memset(kopad, 0, blocksize);
    memcpy(kipad, kptr, klen);
    memcpy(kopad, kptr, klen);

    for (size_t i = 0; i < blocksize; ++i) {
        kipad[i] ^= 0x36;
        kopad[i] ^= 0x5c;
    }

    SHA256_CTX ictx;
    sha256_init(&ictx);
    sha256_update(&ictx, kipad, blocksize);
    sha256_update(&ictx, data, datalen);
    sha256_final(&ictx, khash);

    SHA256_CTX octx;
    sha256_init(&octx);
    sha256_update(&octx, kopad, blocksize);
    sha256_update(&octx, khash, 32);
    sha256_final(&octx, out);

    memset(kipad, 0, sizeof(kipad));
    memset(kopad, 0, sizeof(kopad));
    memset(khash, 0, sizeof(khash));
    memset(tk, 0, sizeof(tk));
}

static void int_to_be32_c(uint32_t n, uint8_t out[4]){
    out[0] = (uint8_t)((n >> 24) & 0xFF);
    out[1] = (uint8_t)((n >> 16) & 0xFF);
    out[2] = (uint8_t)((n >> 8) & 0xFF);
    out[3] = (uint8_t)(n & 0xFF);
}

static void pbkdf2_hmac_sha256_c(const uint8_t* password, size_t passlen,
                                 const uint8_t* salt, size_t saltlen,
                                 uint32_t iterations, uint8_t* out, size_t dkLen){
    const size_t hLen = 32;
    uint32_t l = (uint32_t)((dkLen + hLen - 1) / hLen);
    uint32_t r = (uint32_t)(dkLen - (l - 1) * hLen);
    uint8_t* U = (uint8_t*)malloc(hLen);
    uint8_t* T = (uint8_t*)malloc(hLen);
    uint8_t* saltBlock = (uint8_t*)malloc(saltlen + 4);

    if (!U || !T || !saltBlock) {
        if (U) { memset(U,0,hLen); free(U); }
        if (T) { memset(T,0,hLen); free(T); }
        if (saltBlock) { memset(saltBlock,0,saltlen+4); free(saltBlock); }
        return;
    }

    memcpy(saltBlock, salt, saltlen);

    for (uint32_t i = 1; i <= l; ++i) {
        int_to_be32_c(i, saltBlock + saltlen);
        hmac_sha256(password, passlen, saltBlock, saltlen + 4, U);
        memcpy(T, U, hLen);
        for (uint32_t j = 2; j <= iterations; ++j) {
            hmac_sha256(password, passlen, U, hLen, U);
            for (size_t k = 0; k < hLen; ++k) {
                T[k] ^= U[k];
            }
        }
        size_t offset = (size_t)(i - 1) * hLen;
        size_t clen = (i == l) ? r : hLen;
        memcpy(out + offset, T, clen);
    }

    memset(U, 0, hLen);
    memset(T, 0, hLen);
    memset(saltBlock, 0, saltlen + 4);
    free(U);
    free(T);
    free(saltBlock);
}

static void to_hex_c(const uint8_t* data, size_t len, char* out_hex){
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[2*i]   = hex[(data[i] >> 4) & 0x0F];
        out_hex[2*i+1] = hex[data[i] & 0x0F];
    }
    out_hex[2*len] = '\0';
}

char* compute_hash(const char* password, const char* salt){
    const uint32_t iterations = 200000;
    const size_t dkLen = 32;
    uint8_t dk[32];
    pbkdf2_hmac_sha256_c((const uint8_t*)password, strlen(password),
                         (const uint8_t*)salt, strlen(salt),
                         iterations, dk, dkLen);
    char* hex = (char*)malloc(dkLen * 2 + 1);
    if (!hex) {
        return NULL;
    }
    to_hex_c(dk, dkLen, hex);
    // Clear sensitive derived key
    memset(dk, 0, sizeof(dk));
    return hex; // caller must free
}

int main(void){
    char* h1 = compute_hash("password", "salt");
    char* h2 = compute_hash("correct horse battery staple", "somesalt");
    char* h3 = compute_hash("P@ssw0rd!", "NaCl");
    char* h4 = compute_hash("", "salt");
    char* h5 = compute_hash("password", "");

    if (h1) { printf("%s\n", h1); free(h1); }
    if (h2) { printf("%s\n", h2); free(h2); }
    if (h3) { printf("%s\n", h3); free(h3); }
    if (h4) { printf("%s\n", h4); free(h4); }
    if (h5) { printf("%s\n", h5); free(h5); }

    return 0;
}