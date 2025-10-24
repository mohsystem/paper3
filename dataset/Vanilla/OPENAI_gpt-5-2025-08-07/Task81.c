#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Minimal SHA-256 implementation (public domain style)

typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

static const uint32_t k256[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x,uint32_t y,uint32_t z){return (x & y) ^ (~x & z);}
static inline uint32_t maj(uint32_t x,uint32_t y,uint32_t z){return (x & y) ^ (x & z) ^ (y & z);}
static inline uint32_t ep0(uint32_t x){return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22);}
static inline uint32_t ep1(uint32_t x){return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25);}
static inline uint32_t sig0(uint32_t x){return rotr(x,7) ^ rotr(x,18) ^ (x >> 3);}
static inline uint32_t sig1(uint32_t x){return rotr(x,17) ^ rotr(x,19) ^ (x >> 10);}

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[])
{
    uint32_t a,b,c,d,e,f,g,h,m[64],t1,t2;
    for (uint32_t i=0,j=0;i<16;++i, j+=4)
        m[i] = (uint32_t(data[j])<<24) | (uint32_t(data[j+1])<<16) | (uint32_t(data[j+2])<<8) | (uint32_t(data[j+3]));
    for (uint32_t i=16;i<64;++i)
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (uint32_t i=0;i<64;++i){
        t1 = h + ep1(e) + ch(e,f,g) + k256[i] + m[i];
        t2 = ep0(a) + maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_init(SHA256_CTX *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len)
{
    for (size_t i=0; i<len; ++i){
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[])
{
    uint32_t i = ctx->datalen;

    // Pad
    if (ctx->datalen < 56){
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    ctx->bitlen += (uint64_t)ctx->datalen * 8ull;
    ctx->data[63] = (uint8_t)(ctx->bitlen      );
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8 );
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);

    for (i=0; i<4; ++i){
        hash[i    ] = (uint8_t)((ctx->state[0] >> (24 - i*8)) & 0xff);
        hash[i + 4] = (uint8_t)((ctx->state[1] >> (24 - i*8)) & 0xff);
        hash[i + 8] = (uint8_t)((ctx->state[2] >> (24 - i*8)) & 0xff);
        hash[i +12] = (uint8_t)((ctx->state[3] >> (24 - i*8)) & 0xff);
        hash[i +16] = (uint8_t)((ctx->state[4] >> (24 - i*8)) & 0xff);
        hash[i +20] = (uint8_t)((ctx->state[5] >> (24 - i*8)) & 0xff);
        hash[i +24] = (uint8_t)((ctx->state[6] >> (24 - i*8)) & 0xff);
        hash[i +28] = (uint8_t)((ctx->state[7] >> (24 - i*8)) & 0xff);
    }
}

static char* to_hex(const uint8_t* data, size_t len) {
    static const char hex[] = "0123456789abcdef";
    char* out = (char*)malloc(len*2 + 1);
    if (!out) return NULL;
    for (size_t i=0;i<len;++i){
        out[2*i]   = hex[(data[i] >> 4) & 0x0F];
        out[2*i+1] = hex[data[i] & 0x0F];
    }
    out[len*2] = '\0';
    return out;
}

static int equals_hex_ignore_case(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    for (size_t i=0;i<la;++i){
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (tolower(ca) != tolower(cb)) return 0;
    }
    return 1;
}

int matchesKnownHash(const uint8_t* certBytes, size_t len, const char* knownHashHex, const char* algorithm) {
    if (!algorithm) return 0;
    // accept "sha256" or "sha-256"
    int ok = 0;
    {
        size_t n = strlen(algorithm);
        char* lower = (char*)malloc(n+1);
        if (!lower) return 0;
        for (size_t i=0;i<n;++i) lower[i] = (char)tolower((unsigned char)algorithm[i]);
        lower[n] = '\0';
        if (strcmp(lower, "sha256")==0 || strcmp(lower, "sha-256")==0) ok = 1;
        free(lower);
    }
    if (!ok) return 0;

    SHA256_CTX ctx;
    sha256_init(&ctx);
    if (len > 0) sha256_update(&ctx, certBytes, len);
    uint8_t digest[32];
    sha256_final(&ctx, digest);
    char* hex = to_hex(digest, 32);
    int res = equals_hex_ignore_case(hex, knownHashHex);
    free(hex);
    return res;
}

int main() {
    const char* algo = "SHA-256";

    const uint8_t cert1[] = {'a','b','c'};
    const char* hash1 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    printf("%s\n", matchesKnownHash(cert1, sizeof(cert1), hash1, algo) ? "true" : "false");

    const uint8_t* cert2 = (const uint8_t*)"";
    const char* hash2 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    printf("%s\n", matchesKnownHash(cert2, 0, hash2, algo) ? "true" : "false");

    const uint8_t cert3[] = {'h','e','l','l','o'};
    const char* hash3 = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
    printf("%s\n", matchesKnownHash(cert3, sizeof(cert3), hash3, algo) ? "true" : "false");

    const uint8_t cert4[] = {'t','e','s','t'};
    const char* wrong = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
    printf("%s\n", matchesKnownHash(cert4, sizeof(cert4), wrong, algo) ? "true" : "false");

    const uint8_t cert5[] = {0x00,0x01,0x02,0x03};
    printf("%s\n", matchesKnownHash(cert5, sizeof(cert5), hash1, algo) ? "true" : "false");

    return 0;
}