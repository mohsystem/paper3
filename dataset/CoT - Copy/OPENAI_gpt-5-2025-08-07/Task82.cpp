// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement PBKDF2-HMAC-SHA256 to compute salted password hash.
// 2) Security requirements: Use strong KDF with adequate iterations and key length.
// 3) Secure coding generation: Implement SHA-256, HMAC, PBKDF2 carefully; avoid UB; use fixed params.
// 4) Code review: Ensure correct big-endian handling, no buffer overflows, constant sizes.
// 5) Secure code output: 200000 iterations, 32-byte derived key, hex output.

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>

struct SHA256_CTX {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
};

static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

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

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]){
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];

    for (i = 0; i < 16; ++i) {
        m[i] = (uint32_t)data[i*4] << 24 |
               (uint32_t)data[i*4 + 1] << 16 |
               (uint32_t)data[i*4 + 2] << 8 |
               (uint32_t)data[i*4 + 3];
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
        t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
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
    ctx->state[0] = 0x6a09e667ul;
    ctx->state[1] = 0xbb67ae85ul;
    ctx->state[2] = 0x3c6ef372ul;
    ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful;
    ctx->state[5] = 0x9b05688cul;
    ctx->state[6] = 0x1f83d9abul;
    ctx->state[7] = 0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t data[], size_t len){
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    uint32_t i = ctx->datalen;

    // Pad
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        std::memset(ctx->data, 0, 56);
    }

    ctx->bitlen += (uint64_t)ctx->datalen * 8ull;

    // Append length in bits big-endian
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);

    // Output
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xFF;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xFF;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xFF;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xFF;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xFF;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xFF;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xFF;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xFF;
    }

    // Clear sensitive data (basic)
    std::memset(ctx->data, 0, sizeof(ctx->data));
    ctx->datalen = 0;
    ctx->bitlen = 0;
    std::memset(ctx->state, 0, sizeof(ctx->state));
}

static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    const size_t blocksize = 64;
    uint8_t kopad[blocksize];
    uint8_t kipad[blocksize];
    uint8_t khash[32];
    uint8_t tk[32];

    if (keylen > blocksize) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, keylen);
        sha256_final(&tctx, tk);
        key = tk;
        keylen = 32;
    }

    std::memset(kipad, 0, blocksize);
    std::memset(kopad, 0, blocksize);
    std::memcpy(kipad, key, keylen);
    std::memcpy(kopad, key, keylen);

    for (size_t i = 0; i < blocksize; ++i) {
        kipad[i] ^= 0x36;
        kopad[i] ^= 0x5c;
    }

    // inner hash
    SHA256_CTX ictx;
    sha256_init(&ictx);
    sha256_update(&ictx, kipad, blocksize);
    sha256_update(&ictx, data, datalen);
    sha256_final(&ictx, khash);

    // outer hash
    SHA256_CTX octx;
    sha256_init(&octx);
    sha256_update(&octx, kopad, blocksize);
    sha256_update(&octx, khash, 32);
    sha256_final(&octx, out);

    // Clear sensitive
    std::memset(kipad, 0, sizeof(kipad));
    std::memset(kopad, 0, sizeof(kopad));
    std::memset(khash, 0, sizeof(khash));
    std::memset(tk, 0, sizeof(tk));
}

static void int_to_be32(uint32_t n, uint8_t out[4]) {
    out[0] = (uint8_t)((n >> 24) & 0xFF);
    out[1] = (uint8_t)((n >> 16) & 0xFF);
    out[2] = (uint8_t)((n >> 8) & 0xFF);
    out[3] = (uint8_t)(n & 0xFF);
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations, uint8_t* out, size_t dkLen){
    const size_t hLen = 32;
    uint32_t l = (uint32_t)((dkLen + hLen - 1) / hLen);
    uint32_t r = (uint32_t)(dkLen - (l - 1) * hLen);
    std::vector<uint8_t> U(hLen);
    std::vector<uint8_t> T(hLen);
    std::vector<uint8_t> saltBlock(saltlen + 4);

    std::memcpy(saltBlock.data(), salt, saltlen);

    for (uint32_t i = 1; i <= l; ++i) {
        int_to_be32(i, saltBlock.data() + saltlen);
        hmac_sha256(password, passlen, saltBlock.data(), saltlen + 4, U.data());
        std::memcpy(T.data(), U.data(), hLen);
        for (uint32_t j = 2; j <= iterations; ++j) {
            hmac_sha256(password, passlen, U.data(), hLen, U.data());
            for (size_t k = 0; k < hLen; ++k) {
                T[k] ^= U[k];
            }
        }
        size_t offset = (size_t)(i - 1) * hLen;
        size_t clen = (i == l) ? r : hLen;
        std::memcpy(out + offset, T.data(), clen);
    }

    // Clear sensitive
    std::fill(U.begin(), U.end(), 0);
    std::fill(T.begin(), T.end(), 0);
    std::fill(saltBlock.begin(), saltBlock.end(), 0);
}

static std::string to_hex(const uint8_t* data, size_t len){
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << (unsigned)(data[i]);
    }
    return oss.str();
}

std::string compute_hash(const std::string& password, const std::string& salt){
    const uint32_t iterations = 200000;
    const size_t dkLen = 32;
    std::vector<uint8_t> dk(dkLen);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       reinterpret_cast<const uint8_t*>(salt.data()), salt.size(),
                       iterations, dk.data(), dkLen);
    std::string hex = to_hex(dk.data(), dkLen);
    // Clear sensitive
    std::fill(dk.begin(), dk.end(), 0);
    return hex;
}

int main(){
    std::cout << compute_hash("password", "salt") << "\n";
    std::cout << compute_hash("correct horse battery staple", "somesalt") << "\n";
    std::cout << compute_hash("P@ssw0rd!", "NaCl") << "\n";
    std::cout << compute_hash("", "salt") << "\n";
    std::cout << compute_hash("password", "") << "\n";
    return 0;
}