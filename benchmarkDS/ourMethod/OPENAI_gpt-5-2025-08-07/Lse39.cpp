#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stdexcept>

static const size_t SALT_LEN = 16;
static const size_t HASH_LEN = 32; // 256-bit
static const uint32_t ITERATIONS = 210000;

// --------- Utility: constant-time compare ----------
bool constant_time_equals(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }
    return diff == 0;
}

// --------- SHA-256 implementation (from scratch) ----------
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
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

static inline uint32_t ROTR32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x) { return ROTR32(x, 2) ^ ROTR32(x, 13) ^ ROTR32(x, 22); }
static inline uint32_t EP1(uint32_t x) { return ROTR32(x, 6) ^ ROTR32(x, 11) ^ ROTR32(x, 25); }
static inline uint32_t SIG0(uint32_t x) { return ROTR32(x, 7) ^ ROTR32(x, 18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x) { return ROTR32(x, 17) ^ ROTR32(x, 19) ^ (x >> 10); }

void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (uint32_t)data[i*4] << 24 |
               (uint32_t)data[i*4 + 1] << 16 |
               (uint32_t)data[i*4 + 2] << 8 |
               (uint32_t)data[i*4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
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
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
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

void sha256_init(SHA256_CTX* ctx) {
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

void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    uint32_t i = ctx->datalen;

    // Pad
    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;

    ctx->bitlen += ctx->datalen * 8ull;
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
        for (int j = 0; j < 8; ++j) {
            hash[i + j*4] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xff);
        }
    }
}

// --------- HMAC-SHA256 ----------
void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t len, uint8_t out[32]) {
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t tk[32];

    if (keylen > 64) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, keylen);
        sha256_final(&tctx, tk);
        key = tk;
        keylen = 32;
    }

    std::fill(std::begin(k_ipad), std::end(k_ipad), 0x36);
    std::fill(std::begin(k_opad), std::end(k_opad), 0x5c);

    for (size_t i = 0; i < keylen; ++i) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    SHA256_CTX ctx;
    uint8_t inner[32];

    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, inner);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, inner, 32);
    sha256_final(&ctx, out);

    // zero sensitive buffers
    std::fill(std::begin(tk), std::end(tk), 0);
    std::fill(std::begin(inner), std::end(inner), 0);
    std::fill(std::begin(k_ipad), std::end(k_ipad), 0);
    std::fill(std::begin(k_opad), std::end(k_opad), 0);
}

// --------- PBKDF2-HMAC-SHA256 ----------
void pbkdf2_hmac_sha256(const uint8_t* password, size_t plen, const uint8_t* salt, size_t slen,
                        uint32_t iterations, uint8_t* out, size_t dklen) {
    uint32_t block_count = (uint32_t)((dklen + 31) / 32);
    std::vector<uint8_t> U(32);
    std::vector<uint8_t> T(32);
    std::vector<uint8_t> salt_block(slen + 4);

    salt_block.assign(salt, salt + slen);
    for (uint32_t i = 1; i <= block_count; ++i) {
        salt_block.resize(slen + 4);
        salt_block[slen]     = (uint8_t)(i >> 24);
        salt_block[slen + 1] = (uint8_t)(i >> 16);
        salt_block[slen + 2] = (uint8_t)(i >> 8);
        salt_block[slen + 3] = (uint8_t)(i);

        hmac_sha256(password, plen, salt_block.data(), salt_block.size(), U.data());
        std::copy(U.begin(), U.end(), T.begin());

        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, plen, U.data(), U.size(), U.data());
            for (size_t k = 0; k < 32; ++k) T[k] ^= U[k];
        }

        size_t offset = (i - 1) * 32;
        size_t clen = std::min((size_t)32, dklen - offset);
        std::copy(T.begin(), T.begin() + clen, out + offset);
    }
    std::fill(U.begin(), U.end(), 0);
    std::fill(T.begin(), T.end(), 0);
    std::fill(salt_block.begin(), salt_block.end(), 0);
}

// --------- Random bytes ----------
void random_bytes(uint8_t* buf, size_t len) {
    std::random_device rd;
    for (size_t i = 0; i < len; ++i) {
        buf[i] = static_cast<uint8_t>(rd());
    }
}

// --------- Input validation ----------
bool is_valid_username(const std::string& u) {
    if (u.size() < 3 || u.size() > 64) return false;
    for (char c : u) {
        if (!( (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}
bool is_valid_password_input(const std::string& p) {
    return p.size() >= 8 && p.size() <= 128;
}

// --------- User store ----------
struct User {
    std::string username;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
    uint32_t iterations;
};

static std::vector<User> USERS;

bool create_user(const std::string& username, const std::string& password) {
    if (!is_valid_username(username) || !is_valid_password_input(password)) return false;
    User u;
    u.username = username;
    u.salt.resize(SALT_LEN);
    random_bytes(u.salt.data(), u.salt.size());
    u.hash.resize(HASH_LEN);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       u.salt.data(), u.salt.size(), ITERATIONS, u.hash.data(), u.hash.size());
    u.iterations = ITERATIONS;
    USERS.push_back(u);
    return true;
}

bool verify_login(const std::string& username, const std::string& password) {
    if (!is_valid_username(username) || !is_valid_password_input(password)) return false;
    for (const auto& u : USERS) {
        if (u.username == username) {
            std::vector<uint8_t> computed(HASH_LEN);
            pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                               u.salt.data(), u.salt.size(), u.iterations, computed.data(), computed.size());
            bool ok = constant_time_equals(computed, u.hash);
            std::fill(computed.begin(), computed.end(), 0);
            return ok;
        }
    }
    return false;
}

// Public API
std::string login(const std::string& username, const std::string& password) {
    if (verify_login(username, password)) {
        return "Redirect:/session";
    }
    return "Invalid username or password.";
}

// Generate a strong password (for test only; do not print secrets)
std::string generate_strong_password(size_t length = 16) {
    const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string lower = "abcdefghijklmnopqrstuvwxyz";
    const std::string digits = "0123456789";
    const std::string special = "!@#$%^&*()-_=+[]{}:,.?/~";
    const std::string all = upper + lower + digits + special;
    std::random_device rd;

    std::string pw;
    pw.reserve(length);
    pw.push_back(upper[rd() % upper.size()]);
    pw.push_back(lower[rd() % lower.size()]);
    pw.push_back(digits[rd() % digits.size()]);
    pw.push_back(special[rd() % special.size()]);
    while (pw.size() < length) {
        pw.push_back(all[rd() % all.size()]);
    }
    std::shuffle(pw.begin(), pw.end(), std::mt19937{std::random_device{}()});
    pw.resize(length);
    return pw;
}

int main() {
    std::string user = "alice";
    std::string correct = generate_strong_password(16);
    if (!create_user(user, correct)) {
        std::cout << "User setup failed\n";
        return 1;
    }

    // 5 test cases
    std::vector<std::pair<std::string, std::string>> tests = {
        {user, correct},            // correct
        {user, "WrongPass!123"},    // wrong password
        {"bob", "Anything123!"},    // non-existent
        {"", "short"},              // invalid username
        {"al", "Password!234"}      // too short username
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i+1) << ": " << login(tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}