#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <random>
#include <algorithm>

struct UserRecord {
    std::string username;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
};

struct InMemoryDB {
    std::vector<UserRecord> users;
    bool insert(const UserRecord& rec) {
        for (const auto& u : users) {
            if (u.username == rec.username) return false;
        }
        users.push_back(rec);
        return true;
    }
    const UserRecord* get(const std::string& username) const {
        for (const auto& u : users) {
            if (u.username == username) return &u;
        }
        return nullptr;
    }
    size_t size() const { return users.size(); }
};

static inline bool isValidUsername(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

static inline bool isValidPassword(const std::string& p) {
    return p.size() >= 8;
}

static void secure_bzero(void* v, size_t n) {
    volatile uint8_t* p = (volatile uint8_t*)v;
    while (n--) *p++ = 0;
}

/* SHA-256 implementation */
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

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

#define ROTR(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR((x),2) ^ ROTR((x),13) ^ ROTR((x),22))
#define EP1(x) (ROTR((x),6) ^ ROTR((x),11) ^ ROTR((x),25))
#define SIG0(x) (ROTR((x),7) ^ ROTR((x),18) ^ ((x) >> 3))
#define SIG1(x) (ROTR((x),17) ^ ROTR((x),19) ^ ((x) >> 10))

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; i++) {
        m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 |
               (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
    }
    for (int i = 16; i < 64; i++) {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i = 0; i < 64; i++) {
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + k256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]) {
    uint64_t bitlen = ctx->bitlen + ctx->datalen * 8;
    size_t i = ctx->datalen;

    // Pad
    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0x00;
    // Append length
    for (int j = 7; j >= 0; j--) {
        ctx->data[i++] = (uint8_t)((bitlen >> (j*8)) & 0xFF);
    }
    sha256_transform(ctx, ctx->data);

    for (int j = 0; j < 8; j++) {
        hash[j*4]   = (uint8_t)((ctx->state[j] >> 24) & 0xFF);
        hash[j*4+1] = (uint8_t)((ctx->state[j] >> 16) & 0xFF);
        hash[j*4+2] = (uint8_t)((ctx->state[j] >> 8) & 0xFF);
        hash[j*4+3] = (uint8_t)(ctx->state[j] & 0xFF);
    }
    secure_bzero(ctx, sizeof(*ctx));
}

static void hmac_sha256(const uint8_t* key, size_t keylen,
                        const uint8_t* data, size_t datalen,
                        uint8_t out[32]) {
    uint8_t k_ipad[64], k_opad[64];
    uint8_t tk[32];
    if (keylen > 64) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, keylen);
        sha256_final(&c, tk);
        key = tk;
        keylen = 32;
    }
    std::fill(k_ipad, k_ipad+64, 0x36);
    std::fill(k_opad, k_opad+64, 0x5c);
    for (size_t i = 0; i < keylen; i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }
    uint8_t inner_hash[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, datalen);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, inner_hash, 32);
    sha256_final(&ctx, out);

    secure_bzero(tk, sizeof(tk));
    secure_bzero(k_ipad, sizeof(k_ipad));
    secure_bzero(k_opad, sizeof(k_opad));
    secure_bzero(inner_hash, sizeof(inner_hash));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* out, size_t outlen) {
    size_t hashlen = 32;
    uint32_t block_count = (uint32_t)((outlen + hashlen - 1) / hashlen);
    std::vector<uint8_t> U(hashlen);
    std::vector<uint8_t> T(hashlen);
    std::vector<uint8_t> salt_block(saltlen + 4);

    std::memcpy(salt_block.data(), salt, saltlen);
    for (uint32_t i = 1; i <= block_count; i++) {
        salt_block[saltlen] = (uint8_t)((i >> 24) & 0xFF);
        salt_block[saltlen+1] = (uint8_t)((i >> 16) & 0xFF);
        salt_block[saltlen+2] = (uint8_t)((i >> 8) & 0xFF);
        salt_block[saltlen+3] = (uint8_t)(i & 0xFF);

        hmac_sha256(password, passlen, salt_block.data(), saltlen + 4, U.data());
        std::memcpy(T.data(), U.data(), hashlen);

        for (uint32_t j = 1; j < iterations; j++) {
            hmac_sha256(password, passlen, U.data(), hashlen, U.data());
            for (size_t k = 0; k < hashlen; k++) {
                T[k] ^= U[k];
            }
        }
        size_t offset = (size_t)(i - 1) * hashlen;
        size_t to_copy = std::min(hashlen, outlen - offset);
        std::memcpy(out + offset, T.data(), to_copy);
    }
    secure_bzero(U.data(), U.size());
    secure_bzero(T.data(), T.size());
    secure_bzero(salt_block.data(), salt_block.size());
}

static std::vector<uint8_t> generateSalt(size_t len = 16) {
    std::vector<uint8_t> s(len);
    std::random_device rd;
    for (size_t i = 0; i < len; i++) s[i] = static_cast<uint8_t>(rd());
    return s;
}

static std::vector<uint8_t> hashPassword(const std::string& password,
                                         const std::vector<uint8_t>& salt,
                                         uint32_t iterations = 100000,
                                         size_t dkLen = 32) {
    std::vector<uint8_t> out(dkLen);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       salt.data(), salt.size(), iterations, out.data(), out.size());
    return out;
}

static UserRecord createUserRecord(const std::string& username, const std::string& password) {
    auto salt = generateSalt(16);
    auto hash = hashPassword(password, salt, 100000, 32);
    return UserRecord{username, std::move(salt), std::move(hash)};
}

static bool registerUser(InMemoryDB& db, const std::string& username, const std::string& password) {
    if (!isValidUsername(username) || !isValidPassword(password)) return false;
    UserRecord rec = createUserRecord(username, password);
    return db.insert(rec);
}

static std::string toHex(const std::vector<uint8_t>& v) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(v.size()*2);
    for (uint8_t b : v) {
        s.push_back(hex[(b >> 4) & 0xF]);
        s.push_back(hex[b & 0xF]);
    }
    return s;
}

int main() {
    InMemoryDB db;
    std::vector<std::pair<std::string,std::string>> tests = {
        {"alice_01", "S3cur3P@ssw0rd!"},
        {"bob_user", "Another$trong1"},
        {"charlie_3", "passw0rdLongEnough"},
        {"bob_user", "Another$trong1"},
        {"eve_user", "Y3t@An0th3rP@ss"}
    };
    for (auto& t : tests) {
        bool ok = registerUser(db, t.first, t.second);
        std::cout << "Register '" << t.first << "': " << (ok ? "true" : "false") << "\n";
        auto rec = db.get(t.first);
        if (rec) {
            std::cout << "  salt(hex)=" << toHex(rec->salt) << "\n";
            std::cout << "  hash(hex)=" << toHex(rec->hash) << "\n";
        }
    }
    std::cout << "Total users stored: " << db.size() << "\n";
    return 0;
}