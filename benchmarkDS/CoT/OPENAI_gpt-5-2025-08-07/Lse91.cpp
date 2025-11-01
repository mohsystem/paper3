// Chain-of-Through process:
// 1) Problem understanding: Implement registration logic with input validation, password hashing with salt, and storage in an in-memory DB. Provide a function that accepts parameters and returns output. Include 5 tests.
// 2) Security requirements: Use SHA-256 with per-user random salt and many iterations (PBKDF2 is not in standard C++, so we approximate with iterative salted SHA-256). Validate inputs.
// 3) Secure coding generation: Implement SHA-256, salt generation via random_device, safe string handling, bounded inputs.
// 4) Code review: Ensure no UB, safe memory usage, no sensitive logging.
// 5) Secure code output: Final code below.

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <random>
#include <algorithm>

// Simple in-memory DB: username -> "salt_hex:hash_hex"
static std::unordered_map<std::string, std::string> DB;

static bool valid_username(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

static bool valid_password(const std::string& p) {
    return p.size() >= 8 && p.size() <= 1024;
}

// SHA-256 implementation
struct SHA256_CTX {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
};

#define SHA256_ROTR(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define SHA256_CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x) (SHA256_ROTR(x,2) ^ SHA256_ROTR(x,13) ^ SHA256_ROTR(x,22))
#define SHA256_EP1(x) (SHA256_ROTR(x,6) ^ SHA256_ROTR(x,11) ^ SHA256_ROTR(x,25))
#define SHA256_SIG0(x) (SHA256_ROTR(x,7) ^ SHA256_ROTR(x,18) ^ ((x) >> 3))
#define SHA256_SIG1(x) (SHA256_ROTR(x,17) ^ SHA256_ROTR(x,19) ^ ((x) >> 10))

static const uint32_t sha256_k[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = (data[j] << 24) | (data[j+1] << 16) | (data[j+2] << 8) | (data[j+3]);
    }
    for (; i < 64; ++i) {
        m[i] = SHA256_SIG1(m[i-2]) + m[i-7] + SHA256_SIG0(m[i-15]) + m[i-16];
    }

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + SHA256_EP1(e) + SHA256_CH(e,f,g) + sha256_k[i] + m[i];
        t2 = SHA256_EP0(a) + SHA256_MAJ(a,b,c);
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

static void sha256_update(SHA256_CTX* ctx, const uint8_t data[], size_t len) {
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

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[]) {
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
    ctx->bitlen += ctx->datalen * 8ull;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xff;
    }
}

static std::vector<uint8_t> sha256_bytes(const std::vector<uint8_t>& data) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data.data(), data.size());
    std::vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

static std::string to_hex(const std::vector<uint8_t>& data) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(data.size() * 2);
    for (auto b : data) {
        s.push_back(hex[(b >> 4) & 0xF]);
        s.push_back(hex[b & 0xF]);
    }
    return s;
}

static std::string to_hex_bytes(const uint8_t* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        uint8_t b = data[i];
        s.push_back(hex[(b >> 4) & 0xF]);
        s.push_back(hex[b & 0xF]);
    }
    return s;
}

static std::vector<uint8_t> random_bytes(size_t n) {
    std::vector<uint8_t> out(n);
    std::random_device rd;
    for (size_t i = 0; i < n; ++i) out[i] = static_cast<uint8_t>(rd());
    return out;
}

// Iterative salted SHA-256: H0 = SHA256(salt || password), Hi = SHA256(salt || H(i-1))
static std::string hash_password(const std::string& password, const std::vector<uint8_t>& salt, int iterations = 200000) {
    std::vector<uint8_t> buf;
    buf.reserve(salt.size() + password.size());
    buf.insert(buf.end(), salt.begin(), salt.end());
    buf.insert(buf.end(), password.begin(), password.end());
    std::vector<uint8_t> h = sha256_bytes(buf);
    for (int i = 1; i < iterations; ++i) {
        std::vector<uint8_t> nxt;
        nxt.reserve(salt.size() + h.size());
        nxt.insert(nxt.end(), salt.begin(), salt.end());
        nxt.insert(nxt.end(), h.begin(), h.end());
        h = sha256_bytes(nxt);
    }
    return to_hex_bytes(salt.data(), salt.size()) + ":" + to_hex(h);
}

std::string register_user(const std::string& username, const std::string& password) {
    if (!valid_username(username)) return "error: invalid username";
    if (!valid_password(password)) return "error: invalid password length";
    if (DB.find(username) != DB.end()) return "error: username already exists";
    auto salt = random_bytes(16);
    std::string record = hash_password(password, salt, 200000);
    DB[username] = record;
    return "ok";
}

int main() {
    std::cout << register_user("user_one", "StrongPass123!") << "\n";   // ok
    std::cout << register_user("user_one", "AnotherPass123!") << "\n";  // duplicate
    std::cout << register_user("u!", "bad") << "\n";                    // invalid username
    std::cout << register_user("shortpwd", "123") << "\n";              // short password
    std::cout << register_user("second_user", "Another#Strong$Pass456") << "\n"; // ok
    return 0;
}