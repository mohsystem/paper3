#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstring>
#include <random>
#include <algorithm>

static const size_t SALT_LEN = 16;
static const size_t KEY_LEN = 32; // 256-bit
static const uint32_t ITERATIONS = 210000;

// ---- Secure zeroization ----
static void secure_memzero(void* ptr, size_t len) {
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

// ---- SHA-256 (public domain style minimal implementation) ----
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

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

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32U - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x) { return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static inline uint32_t ep1(uint32_t x) { return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static inline uint32_t sig0(uint32_t x) { return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x) { return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]) {
    uint32_t m[64];
    for (uint32_t i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j+1] << 16) |
               ((uint32_t)data[j+2] << 8) | (uint32_t)data[j+3];
    }
    for (uint32_t i = 16; i < 64; ++i) {
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
    }

    uint32_t a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3];
    uint32_t e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];

    for (uint32_t i = 0; i < 64; ++i) {
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + K256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
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
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    uint32_t i = ctx->datalen;

    // Pad
    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;

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
    // Clear sensitive state
    secure_memzero(ctx, sizeof(*ctx));
}

// ---- HMAC-SHA256 ----
static void hmac_sha256(const uint8_t* key, size_t keylen,
                        const uint8_t* data, size_t datalen,
                        uint8_t out[32]) {
    const size_t blocksize = 64;
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];

    if (keylen > blocksize) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, keylen);
        sha256_final(&c, tk);
        key = tk;
        keylen = 32;
    }

    memset(k_ipad, 0x36, blocksize);
    memset(k_opad, 0x5c, blocksize);
    for (size_t i = 0; i < keylen; ++i) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    uint8_t inner_hash[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, blocksize);
    sha256_update(&ctx, data, datalen);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, blocksize);
    sha256_update(&ctx, inner_hash, 32);
    sha256_final(&ctx, out);

    secure_memzero(k_ipad, sizeof(k_ipad));
    secure_memzero(k_opad, sizeof(k_opad));
    secure_memzero(tk, sizeof(tk));
    secure_memzero(inner_hash, sizeof(inner_hash));
}

// ---- PBKDF2-HMAC-SHA256 ----
static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* out, size_t outlen) {
    uint32_t block_count = (uint32_t)((outlen + 31u) / 32u);
    std::vector<uint8_t> asalt(salt, salt + saltlen);
    asalt.resize(saltlen + 4);

    uint8_t u[32];
    uint8_t t[32];

    for (uint32_t i = 1; i <= block_count; ++i) {
        asalt[saltlen + 0] = (uint8_t)((i >> 24) & 0xFF);
        asalt[saltlen + 1] = (uint8_t)((i >> 16) & 0xFF);
        asalt[saltlen + 2] = (uint8_t)((i >> 8) & 0xFF);
        asalt[saltlen + 3] = (uint8_t)(i & 0xFF);

        hmac_sha256(password, passlen, asalt.data(), asalt.size(), u);
        memcpy(t, u, 32);
        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, passlen, u, 32, u);
            for (size_t k = 0; k < 32; ++k) {
                t[k] ^= u[k];
            }
        }
        size_t offset = (size_t)(i - 1) * 32u;
        size_t clen = std::min((size_t)32, outlen - offset);
        memcpy(out + offset, t, clen);
    }
    secure_memzero(u, sizeof(u));
    secure_memzero(t, sizeof(t));
    secure_memzero(asalt.data(), asalt.size());
}

// ---- Constant-time compare ----
static bool ct_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t diff = 0;
    for (size_t i = 0; i < n; ++i) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }
    return diff == 0;
}

// ---- Random bytes (prefer non-deterministic random_device) ----
static bool get_random_bytes(uint8_t* out, size_t len) {
    std::random_device rd;
    for (size_t i = 0; i < len; ++i) {
        out[i] = static_cast<uint8_t>(rd());
    }
    return true;
}

// ---- Username validation ----
static bool valid_username(const std::string& u) {
    if (u.empty() || u.size() > 64) return false;
    for (char c : u) {
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

static bool valid_password_for_login(const std::string& p) {
    return !p.empty() && p.size() <= 1024;
}

static bool password_strong(const std::string& p) {
    if (p.size() < 12 || p.size() > 1024) return false;
    bool U=false,L=false,D=false,S=false;
    for (unsigned char c : p) {
        if (c >= 'A' && c <= 'Z') U = true;
        else if (c >= 'a' && c <= 'z') L = true;
        else if (c >= '0' && c <= '9') D = true;
        else S = true;
    }
    return U && L && D && S;
}

struct UserRecord {
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
    uint32_t iterations;
};

static std::unordered_map<std::string, UserRecord> STORE;

static void register_user(const std::string& username, const std::string& password) {
    if (!valid_username(username)) {
        throw std::runtime_error("Invalid username");
    }
    if (!password_strong(password)) {
        throw std::runtime_error("Password does not meet policy");
    }
    std::vector<uint8_t> salt(SALT_LEN);
    if (!get_random_bytes(salt.data(), salt.size())) {
        throw std::runtime_error("RNG failure");
    }
    std::vector<uint8_t> dk(KEY_LEN);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       salt.data(), salt.size(), ITERATIONS, dk.data(), dk.size());
    STORE[username] = UserRecord{salt, dk, ITERATIONS};
}

static std::string check_mod(const std::string& username, const std::string& password) {
    if (!valid_username(username) || !valid_password_for_login(password)) {
        return "false";
    }
    auto it = STORE.find(username);
    if (it == STORE.end()) {
        return "false";
    }
    const UserRecord& rec = it->second;
    std::vector<uint8_t> dk(rec.hash.size());
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       rec.salt.data(), rec.salt.size(), rec.iterations, dk.data(), dk.size());
    bool ok = (dk.size() == rec.hash.size()) && ct_equal(dk.data(), rec.hash.data(), dk.size());
    secure_memzero(dk.data(), dk.size());
    return ok ? "true" : "false";
}

int main() {
    // Initialize users
    register_user("alice", "Str0ng!Pass#1");
    register_user("bob", "Another$Pass2");
    register_user("eve", "Complex%Pass3");

    // 5 test cases
    std::cout << check_mod("alice", "Str0ng!Pass#1") << "\n"; // true
    std::cout << check_mod("alice", "wrong") << "\n";         // false
    std::cout << check_mod("unknown", "whatever") << "\n";    // false
    std::cout << check_mod("", "nope") << "\n";               // false
    std::cout << check_mod("bob", "Another$Pass2") << "\n";   // true
    return 0;
}