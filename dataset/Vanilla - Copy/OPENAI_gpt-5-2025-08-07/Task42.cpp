#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>
#include <fstream>
#include <random>

// SHA-256 implementation
struct SHA256Ctx {
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

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

static void sha256_transform(SHA256Ctx &ctx, const uint8_t data[64]) {
    uint32_t a,b,c,d,e,f,g,h,t1,t2,m[64];
    for (uint32_t i=0;i<16;i++) {
        m[i] = (uint32_t)data[i*4] << 24 |
               (uint32_t)data[i*4+1] << 16 |
               (uint32_t)data[i*4+2] << 8 |
               (uint32_t)data[i*4+3];
    }
    for (uint32_t i=16;i<64;i++) {
        uint32_t s0 = rotr(m[i-15],7) ^ rotr(m[i-15],18) ^ (m[i-15] >> 3);
        uint32_t s1 = rotr(m[i-2],17) ^ rotr(m[i-2],19) ^ (m[i-2] >> 10);
        m[i] = m[i-16] + s0 + m[i-7] + s1;
    }
    a = ctx.state[0]; b = ctx.state[1]; c = ctx.state[2]; d = ctx.state[3];
    e = ctx.state[4]; f = ctx.state[5]; g = ctx.state[6]; h = ctx.state[7];
    for (uint32_t i=0;i<64;i++) {
        uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        t1 = h + S1 + ch + K256[i] + m[i];
        uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        t2 = S0 + maj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx.state[0] += a; ctx.state[1] += b; ctx.state[2] += c; ctx.state[3] += d;
    ctx.state[4] += e; ctx.state[5] += f; ctx.state[6] += g; ctx.state[7] += h;
}

static void sha256_init(SHA256Ctx &ctx) {
    ctx.datalen = 0;
    ctx.bitlen = 0;
    ctx.state[0] = 0x6a09e667ul;
    ctx.state[1] = 0xbb67ae85ul;
    ctx.state[2] = 0x3c6ef372ul;
    ctx.state[3] = 0xa54ff53aul;
    ctx.state[4] = 0x510e527ful;
    ctx.state[5] = 0x9b05688cul;
    ctx.state[6] = 0x1f83d9abul;
    ctx.state[7] = 0x5be0cd19ul;
}

static void sha256_update(SHA256Ctx &ctx, const uint8_t *data, size_t len) {
    for (size_t i=0;i<len;i++) {
        ctx.data[ctx.datalen++] = data[i];
        if (ctx.datalen == 64) {
            sha256_transform(ctx, ctx.data);
            ctx.bitlen += 512;
            ctx.datalen = 0;
        }
    }
}

static void sha256_final(SHA256Ctx &ctx, uint8_t hash[32]) {
    size_t i = ctx.datalen;
    // Pad
    ctx.data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx.data[i++] = 0x00;
        sha256_transform(ctx, ctx.data);
        i = 0;
    }
    while (i < 56) ctx.data[i++] = 0x00;
    ctx.bitlen += ctx.datalen * 8ull;
    // Append length in bits
    for (int j=7;j>=0;j--) {
        ctx.data[56 + (7-j)] = (uint8_t)((ctx.bitlen >> (j*8)) & 0xFF);
    }
    sha256_transform(ctx, ctx.data);
    for (i=0;i<8;i++) {
        hash[i*4    ] = (uint8_t)((ctx.state[i] >> 24) & 0xFF);
        hash[i*4 + 1] = (uint8_t)((ctx.state[i] >> 16) & 0xFF);
        hash[i*4 + 2] = (uint8_t)((ctx.state[i] >> 8) & 0xFF);
        hash[i*4 + 3] = (uint8_t)(ctx.state[i] & 0xFF);
    }
}

static void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
    SHA256Ctx ctx;
    sha256_init(ctx);
    sha256_update(ctx, data, len);
    sha256_final(ctx, out);
}

static void fill_random(uint8_t* buf, size_t len) {
    // Try /dev/urandom
    std::ifstream ur("/dev/urandom", std::ios::in | std::ios::binary);
    if (ur.good()) {
        ur.read(reinterpret_cast<char*>(buf), len);
        if ((size_t)ur.gcount() == len) return;
    }
    // Fallback to std::random_device
    std::random_device rd;
    for (size_t i=0;i<len;i++) {
        buf[i] = static_cast<uint8_t>(rd() & 0xFF);
    }
}

static std::vector<uint8_t> derive_hash(const std::string& password, const std::vector<uint8_t>& salt, int iterations) {
    std::vector<uint8_t> first(salt.size() + password.size());
    std::memcpy(first.data(), salt.data(), salt.size());
    std::memcpy(first.data() + salt.size(), password.data(), password.size());
    std::vector<uint8_t> digest(32);
    sha256(first.data(), first.size(), digest.data());
    for (int i=1;i<iterations;i++) {
        std::vector<uint8_t> mix(salt.size() + digest.size());
        std::memcpy(mix.data(), salt.data(), salt.size());
        std::memcpy(mix.data() + salt.size(), digest.data(), digest.size());
        sha256(mix.data(), mix.size(), digest.data());
    }
    return digest;
}

static bool ct_equal(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    unsigned int acc = 0;
    for (size_t i=0;i<a.size();i++) acc |= (unsigned int)(a[i] ^ b[i]);
    return acc == 0;
}

struct User {
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
};

class Task42 {
public:
    bool registerUser(const std::string& username, const std::string& password) {
        if (users.find(username) != users.end()) return false;
        std::vector<uint8_t> salt(16);
        fill_random(salt.data(), salt.size());
        std::vector<uint8_t> h = derive_hash(password, salt, 100000);
        users[username] = User{salt, h};
        return true;
    }
    bool authenticate(const std::string& username, const std::string& password) const {
        auto it = users.find(username);
        if (it == users.end()) return false;
        std::vector<uint8_t> test = derive_hash(password, it->second.salt, 100000);
        return ct_equal(test, it->second.hash);
    }
private:
    std::unordered_map<std::string, User> users;
};

int main() {
    Task42 auth;
    std::cout << (auth.registerUser("alice", "password123") ? "1" : "0") << std::endl; // Test 1
    std::cout << (auth.registerUser("bob", "S3cur3!") ? "1" : "0") << std::endl;       // Test 2
    std::cout << (auth.registerUser("alice", "another") ? "1" : "0") << std::endl;     // Test 3 (duplicate)
    std::cout << (auth.authenticate("alice", "password123") ? "1" : "0") << std::endl; // Test 4
    std::cout << (auth.authenticate("alice", "wrong") ? "1" : "0") << std::endl;       // Test 5
    return 0;
}