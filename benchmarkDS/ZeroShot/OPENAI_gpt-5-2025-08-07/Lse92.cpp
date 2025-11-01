#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <random>

class Lse92 {
public:
    static std::string registerUser(const std::string& username, const std::string& password) {
        if (username.empty()) throw std::invalid_argument("username cannot be empty");
        // Generate salt
        std::vector<uint8_t> salt(16);
        secureRandomBytes(salt.data(), salt.size());
        // Derive key using PBKDF2-HMAC-SHA256
        const uint32_t iterations = 200000;
        std::vector<uint8_t> dk(32);
        pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                           salt.data(), salt.size(), iterations, dk.data(), dk.size());
        std::string out = username + ":PBKDF2$SHA256$" + std::to_string(iterations) + "$" + toHex(salt) + "$" + toHex(dk);
        // wipe sensitive
        std::fill(dk.begin(), dk.end(), 0);
        std::fill(salt.begin(), salt.end(), 0);
        return out;
    }

private:
    // SHA-256 implementation
    struct SHA256_CTX {
        uint32_t state[8];
        uint64_t bitlen;
        uint8_t data[64];
        size_t datalen;
    };

    static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t ep0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline uint32_t ep1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline uint32_t sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline uint32_t sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

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
            m[i] = (uint32_t)data[i*4] << 24 |
                   (uint32_t)data[i*4 + 1] << 16 |
                   (uint32_t)data[i*4 + 2] << 8 |
                   (uint32_t)data[i*4 + 3];
        }
        for (int i = 16; i < 64; ++i) {
            m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
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
            uint32_t t1 = h + ep1(e) + ch(e, f, g) + K[i] + m[i];
            uint32_t t2 = ep0(a) + maj(a, b, c);
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
            std::memset(k0, 0, sizeof(k0));
            std::memcpy(k0, tmp, 32);
            std::memset(tmp, 0, sizeof(tmp));
        } else {
            std::memset(k0, 0, sizeof(k0));
            std::memcpy(k0, key, keylen);
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

        // wipe
        std::memset(k0, 0, sizeof(k0));
        std::memset(ipad, 0, sizeof(ipad));
        std::memset(opad, 0, sizeof(opad));
        std::memset(tmp, 0, sizeof(tmp));
    }

    static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                                   const uint8_t* salt, size_t saltlen,
                                   uint32_t iterations,
                                   uint8_t* out, size_t dkLen) {
        uint32_t block_count = (uint32_t)((dkLen + 31) / 32);
        std::vector<uint8_t> asalt(saltlen + 4);
        std::memcpy(asalt.data(), salt, saltlen);
        uint8_t u[32], t[32];
        size_t pos = 0;
        for (uint32_t i = 1; i <= block_count; ++i) {
            asalt[saltlen] = (uint8_t)((i >> 24) & 0xff);
            asalt[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
            asalt[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
            asalt[saltlen+3] = (uint8_t)(i & 0xff);

            hmac_sha256(password, passlen, asalt.data(), asalt.size(), u);
            std::memcpy(t, u, 32);
            for (uint32_t j = 1; j < iterations; ++j) {
                hmac_sha256(password, passlen, u, 32, u);
                for (int k = 0; k < 32; ++k) t[k] ^= u[k];
            }
            size_t l = std::min((size_t)32, dkLen - pos);
            std::memcpy(out + pos, t, l);
            pos += l;
        }
        std::memset(u, 0, sizeof(u));
        std::memset(t, 0, sizeof(t));
        std::fill(asalt.begin(), asalt.end(), 0);
    }

    static void secureRandomBytes(uint8_t* buf, size_t len) {
        // Try /dev/urandom
        std::ifstream ur("/dev/urandom", std::ios::in | std::ios::binary);
        if (ur.good()) {
            ur.read(reinterpret_cast<char*>(buf), len);
            if (ur.gcount() == (std::streamsize)len) {
                return;
            }
        }
        // Fallback: std::random_device
        std::random_device rd;
        for (size_t i = 0; i < len; ++i) {
            buf[i] = static_cast<uint8_t>(rd());
        }
    }

    static std::string toHex(const std::vector<uint8_t>& data) {
        static const char* hex = "0123456789abcdef";
        std::string s;
        s.resize(data.size() * 2);
        for (size_t i = 0; i < data.size(); ++i) {
            s[2*i] = hex[(data[i] >> 4) & 0xF];
            s[2*i+1] = hex[data[i] & 0xF];
        }
        return s;
    }
};

int main() {
    try {
        std::vector<std::pair<std::string, std::string>> tests = {
            {"alice", "Str0ng!Pass#1"},
            {"bob", "Another$ecret2"},
            {"charlie", "Pa55_word^3"},
            {"dana", "Secur3Key&4"},
            {"eve", "MyP@ssw0rd_5"}
        };
        for (auto& t : tests) {
            std::cout << Lse92::registerUser(t.first, t.second) << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}