#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdio>

// Minimal SHA-256, HMAC-SHA256, and PBKDF2-HMAC-SHA256 implementation

struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static inline uint32_t ROTR(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x) { return ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22); }
static inline uint32_t EP1(uint32_t x) { return ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25); }
static inline uint32_t SIG0(uint32_t x) { return ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x) { return ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10); }

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

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (uint32_t)data[i * 4] << 24 |
               (uint32_t)data[i * 4 + 1] << 16 |
               (uint32_t)data[i * 4 + 2] << 8 |
               (uint32_t)data[i * 4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
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
        uint32_t t1 = h + EP1(e) + CH(e, f, g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a, b, c);
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
    ctx->data[i++] = 0x80;

    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;

    uint64_t total_bits = ctx->bitlen + (ctx->datalen * 8);
    for (int j = 7; j >= 0; --j) {
        ctx->data[56 + (7 - j)] = (uint8_t)((total_bits >> (j * 8)) & 0xff);
    }
    sha256_transform(ctx, ctx->data);

    for (int j = 0; j < 8; ++j) {
        hash[j * 4 + 0] = (uint8_t)((ctx->state[j] >> 24) & 0xff);
        hash[j * 4 + 1] = (uint8_t)((ctx->state[j] >> 16) & 0xff);
        hash[j * 4 + 2] = (uint8_t)((ctx->state[j] >> 8) & 0xff);
        hash[j * 4 + 3] = (uint8_t)((ctx->state[j]) & 0xff);
    }
}

static void secure_memzero(void* v, size_t n) {
    volatile uint8_t* p = static_cast<volatile uint8_t*>(v);
    while (n--) *p++ = 0;
}

static void hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t out[32]) {
    uint8_t kopad[64], kipad[64], keyhash[32];
    const uint8_t* k = key;
    size_t klen = key_len;

    if (klen > 64) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, key_len);
        sha256_final(&tctx, keyhash);
        k = keyhash;
        klen = 32;
    }

    std::memset(kipad, 0x36, 64);
    std::memset(kopad, 0x5c, 64);
    for (size_t i = 0; i < klen; ++i) {
        kipad[i] ^= k[i];
        kopad[i] ^= k[i];
    }

    uint8_t inner[32];
    SHA256_CTX ictx;
    sha256_init(&ictx);
    sha256_update(&ictx, kipad, 64);
    sha256_update(&ictx, data, data_len);
    sha256_final(&ictx, inner);

    SHA256_CTX octx;
    sha256_init(&octx);
    sha256_update(&octx, kopad, 64);
    sha256_update(&octx, inner, 32);
    sha256_final(&octx, out);

    secure_memzero(kipad, sizeof(kipad));
    secure_memzero(kopad, sizeof(kopad));
    secure_memzero(keyhash, sizeof(keyhash));
    secure_memzero(inner, sizeof(inner));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t pass_len,
                               const uint8_t* salt, size_t salt_len,
                               uint32_t iterations,
                               uint8_t* out, size_t dkLen) {
    uint32_t block_count = (uint32_t)((dkLen + 31) / 32);
    std::vector<uint8_t> salt_block(salt_len + 4);
    std::memcpy(salt_block.data(), salt, salt_len);

    for (uint32_t i = 1; i <= block_count; ++i) {
        salt_block[salt_len + 0] = (uint8_t)((i >> 24) & 0xff);
        salt_block[salt_len + 1] = (uint8_t)((i >> 16) & 0xff);
        salt_block[salt_len + 2] = (uint8_t)((i >> 8) & 0xff);
        salt_block[salt_len + 3] = (uint8_t)(i & 0xff);

        uint8_t u[32];
        uint8_t t[32];
        hmac_sha256(password, pass_len, salt_block.data(), salt_len + 4, u);
        std::memcpy(t, u, 32);

        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, pass_len, u, 32, u);
            for (int k = 0; k < 32; ++k) t[k] ^= u[k];
        }

        size_t offset = (i - 1) * 32;
        size_t cp = (dkLen - offset) < 32 ? (dkLen - offset) : 32;
        std::memcpy(out + offset, t, cp);

        secure_memzero(u, sizeof(u));
        secure_memzero(t, sizeof(t));
    }
    secure_memzero(salt_block.data(), salt_block.size());
}

static bool ct_equal(const uint8_t* a, const uint8_t* b, size_t len) {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; ++i) diff |= (a[i] ^ b[i]);
    return diff == 0;
}

static bool get_random_bytes(uint8_t* buf, size_t len) {
    FILE* f = std::fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t r = std::fread(buf, 1, len, f);
    std::fclose(f);
    return r == len;
}

struct UserRecord {
    std::string username;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
    uint32_t iterations;
};

class AuthSystem {
public:
    AuthSystem(uint32_t iterations = 100000, size_t saltLen = 16, size_t dkLen = 32)
        : iterations_(iterations), saltLen_(saltLen), dkLen_(dkLen) {}

    bool registerUser(const std::string& username, const std::string& password) {
        if (!isValidUsername(username) || !isValidPassword(password)) return false;
        if (findUser(username) != nullptr) return false;

        std::vector<uint8_t> salt(saltLen_);
        if (!get_random_bytes(salt.data(), saltLen_)) return false;

        std::vector<uint8_t> dk(dkLen_);
        pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                           salt.data(), salt.size(), iterations_, dk.data(), dk.size());
        UserRecord rec{username, salt, dk, iterations_};
        users_.push_back(rec);
        return true;
    }

    bool authenticate(const std::string& username, const std::string& password) const {
        const UserRecord* rec = findUser(username);
        if (!rec) return false;
        std::vector<uint8_t> dk(rec->hash.size());
        pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                           rec->salt.data(), rec->salt.size(), rec->iterations, dk.data(), dk.size());
        return ct_equal(dk.data(), rec->hash.data(), rec->hash.size());
    }

private:
    bool isValidUsername(const std::string& u) const {
        if (u.size() < 3 || u.size() > 50) return false;
        for (char c : u) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.')) return false;
        }
        return true;
    }
    bool isValidPassword(const std::string& p) const {
        if (p.size() < 8) return false;
        bool up=false, lo=false, di=false, sy=false;
        for (char c : p) {
            if (std::isupper(static_cast<unsigned char>(c))) up = true;
            else if (std::islower(static_cast<unsigned char>(c))) lo = true;
            else if (std::isdigit(static_cast<unsigned char>(c))) di = true;
            else sy = true;
        }
        return up && lo && di && sy;
    }
    const UserRecord* findUser(const std::string& u) const {
        for (const auto& rec : users_) if (rec.username == u) return &rec;
        return nullptr;
    }
    UserRecord* findUser(const std::string& u) {
        for (auto& rec : users_) if (rec.username == u) return &rec;
        return nullptr;
    }

    std::vector<UserRecord> users_;
    uint32_t iterations_;
    size_t saltLen_;
    size_t dkLen_;
};

int main() {
    AuthSystem auth;

    bool t1 = auth.registerUser("alice", "S3cure!Passw0rd");
    std::cout << "Test1 register alice strong pwd: " << (t1 ? "true" : "false") << "\n";

    bool t2 = auth.authenticate("alice", "S3cure!Passw0rd");
    std::cout << "Test2 auth alice correct: " << (t2 ? "true" : "false") << "\n";

    bool t3 = auth.authenticate("alice", "WrongPass!1");
    std::cout << "Test3 auth alice wrong: " << (t3 ? "true" : "false") << "\n";

    bool t4 = auth.registerUser("alice", "An0ther!Pass");
    std::cout << "Test4 duplicate register alice: " << (t4 ? "true" : "false") << "\n";

    bool t5a = auth.registerUser("bob", "short1!");
    bool t5b = auth.registerUser("bob", "G00d!Passw0rd");
    bool t5c = auth.authenticate("bob", "G00d!Passw0rd");
    std::cout << "Test5 bob short register (expect false): " << (t5a ? "true" : "false")
              << ", then good register: " << (t5b ? "true" : "false")
              << ", auth: " << (t5c ? "true" : "false") << "\n";
    return 0;
}