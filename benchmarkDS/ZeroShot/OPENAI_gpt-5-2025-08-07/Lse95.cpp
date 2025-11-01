#include <bits/stdc++.h>
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// Single-file secure registration/login with PBKDF2-HMAC-SHA256 and random salt
class Lse95 {
public:
    static bool registerUser(const std::string& username, const std::string& password) {
        if (!validUsername(username) || !validPassword(password)) return false;
        if (DB().count(username)) return false;
        std::vector<uint8_t> salt(SALT_LEN);
        if (!getRandomBytes(salt.data(), salt.size())) return false;
        std::vector<uint8_t> dk(KEY_LEN);
        if (!pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                                salt.data(), salt.size(), ITERATIONS, dk.data(), dk.size())) return false;
        std::string stored = toHex(dk) + ":" + toHex(salt);
        DB()[username] = stored;
        return true;
    }

    static bool loginUser(const std::string& username, const std::string& password) {
        auto it = DB().find(username);
        if (it == DB().end()) return false;
        auto parts = split(it->second, ':');
        if (parts.size() != 2) return false;
        std::vector<uint8_t> storedHash, salt;
        if (!fromHex(parts[0], storedHash) || !fromHex(parts[1], salt)) return false;
        std::vector<uint8_t> dk(KEY_LEN);
        if (!pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                                salt.data(), salt.size(), ITERATIONS, dk.data(), dk.size())) return false;
        return constTimeEq(storedHash.data(), dk.data(), KEY_LEN);
    }

private:
    static std::unordered_map<std::string, std::string>& DB() {
        static std::unordered_map<std::string, std::string> db;
        return db;
    }

    static bool validUsername(const std::string& u) {
        if (u.size() < 3 || u.size() > 32) return false;
        for (unsigned char c : u) {
            if (!(std::isalnum(c) || c == '_')) return false;
        }
        return true;
    }

    static bool validPassword(const std::string& p) {
        return p.size() >= 8;
    }

    static std::vector<std::string> split(const std::string& s, char sep) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : s) {
            if (c == sep) { out.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        out.push_back(cur);
        return out;
    }

    static std::string toHex(const std::vector<uint8_t>& data) {
        static const char* hex = "0123456789abcdef";
        std::string out;
        out.reserve(data.size() * 2);
        for (uint8_t b : data) {
            out.push_back(hex[b >> 4]);
            out.push_back(hex[b & 0x0F]);
        }
        return out;
    }

    static bool fromHex(const std::string& hex, std::vector<uint8_t>& out) {
        if (hex.size() % 2 != 0) return false;
        out.resize(hex.size() / 2);
        for (size_t i = 0; i < out.size(); ++i) {
            int hi = hexval(hex[2*i]);
            int lo = hexval(hex[2*i+1]);
            if (hi < 0 || lo < 0) return false;
            out[i] = (uint8_t)((hi << 4) | lo);
        }
        return true;
    }

    static int hexval(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    static bool constTimeEq(const uint8_t* a, const uint8_t* b, size_t n) {
        uint8_t diff = 0;
        for (size_t i = 0; i < n; ++i) diff |= (a[i] ^ b[i]);
        return diff == 0;
    }

    // SHA-256 implementation (public-domain style)
    struct SHA256_CTX {
        uint32_t state[8];
        uint64_t bitlen;
        uint8_t data[64];
        size_t datalen;
    };

    static uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static uint32_t bsig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static uint32_t bsig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static uint32_t ssig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static uint32_t ssig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

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
            m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 | (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
        }
        for (int i = 16; i < 64; ++i) {
            m[i] = ssig1(m[i-2]) + m[i-7] + ssig0(m[i-15]) + m[i-16];
        }
        uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
        uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + bsig1(e) + ch(e,f,g) + K[i] + m[i];
            uint32_t t2 = bsig0(a) + maj(a,b,c);
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
            SHA256_CTX tctx;
            sha256_init(&tctx);
            sha256_update(&tctx, key, keylen);
            sha256_final(&tctx, tk);
            key = tk;
            keylen = 32;
        }
        std::memset(kipad, 0, 64);
        std::memset(kopad, 0, 64);
        std::memcpy(kipad, key, keylen);
        std::memcpy(kopad, key, keylen);
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
        std::memset(kipad, 0, 64);
        std::memset(kopad, 0, 64);
        std::memset(tk, 0, 32);
    }

    static bool pbkdf2_hmac_sha256(const uint8_t* pass, size_t passlen,
                                   const uint8_t* salt, size_t saltlen,
                                   uint32_t iterations,
                                   uint8_t* dk, size_t dklen) {
        const size_t hlen = 32;
        size_t l = (dklen + hlen - 1) / hlen;
        size_t r = dklen - (l - 1) * hlen;
        std::vector<uint8_t> U(hlen), T(hlen), saltBlock(saltlen + 4);
        std::memcpy(saltBlock.data(), salt, saltlen);
        for (size_t i = 1; i <= l; ++i) {
            saltBlock[saltlen]   = (uint8_t)((i >> 24) & 0xff);
            saltBlock[saltlen+1] = (uint8_t)((i >> 16) & 0xff);
            saltBlock[saltlen+2] = (uint8_t)((i >> 8) & 0xff);
            saltBlock[saltlen+3] = (uint8_t)(i & 0xff);
            hmac_sha256(pass, passlen, saltBlock.data(), saltlen + 4, U.data());
            std::memcpy(T.data(), U.data(), hlen);
            for (uint32_t j = 1; j < iterations; ++j) {
                hmac_sha256(pass, passlen, U.data(), hlen, U.data());
                for (size_t k = 0; k < hlen; ++k) T[k] ^= U[k];
            }
            size_t toCopy = (i == l) ? r : hlen;
            std::memcpy(dk + (i - 1) * hlen, T.data(), toCopy);
        }
        return true;
    }

    static bool getRandomBytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
        return BCRYPT_SUCCESS(BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG));
#else
        std::ifstream ur("/dev/urandom", std::ios::in | std::ios::binary);
        if (!ur.good()) return false;
        ur.read(reinterpret_cast<char*>(buf), len);
        return (size_t)ur.gcount() == len;
#endif
    }

    static constexpr size_t SALT_LEN = 16;
    static constexpr size_t KEY_LEN = 32;
    static constexpr uint32_t ITERATIONS = 100000;
};

int main() {
    std::cout << "Register alice: " << (Lse95::registerUser("alice", "Str0ngP@ss!") ? "true" : "false") << "\n";
    std::cout << "Login alice (correct): " << (Lse95::loginUser("alice", "Str0ngP@ss!") ? "true" : "false") << "\n";
    std::cout << "Register bob: " << (Lse95::registerUser("bob", "anotherS3cure") ? "true" : "false") << "\n";
    std::cout << "Login bob (wrong): " << (Lse95::loginUser("bob", "wrongpass") ? "true" : "false") << "\n";
    std::cout << "Register alice again (duplicate): " << (Lse95::registerUser("alice", "NewPassw0rd!") ? "true" : "false") << "\n";
    std::cout << "Register charlie_01 (too short): " << (Lse95::registerUser("charlie_01", "short") ? "true" : "false") << "\n";
    std::cout << "Register dave: " << (Lse95::registerUser("dave", "CorrectHorseBatteryStaple1!") ? "true" : "false") << "\n";
    std::cout << "Login dave (correct): " << (Lse95::loginUser("dave", "CorrectHorseBatteryStaple1!") ? "true" : "false") << "\n";
    std::cout << "Login unknown user: " << (Lse95::loginUser("eve", "SomePass123") ? "true" : "false") << "\n";
    return 0;
}