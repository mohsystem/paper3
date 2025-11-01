#include <bits/stdc++.h>
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// Secure registration form with PBKDF2-HMAC-SHA256

// SHA-256 implementation
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static const uint32_t SHA256_K[64] = {
    0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
    0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
    0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
    0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
    0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
    0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
    0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
    0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static inline uint32_t ROTR(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[]){
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
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + SHA256_K[i] + m[i];
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

static void sha256_init(SHA256_CTX* ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
}

static void sha256_update(SHA256_CTX* ctx, const uint8_t* data, size_t len){
    for(size_t i=0;i<len;i++){
        ctx->data[ctx->datalen++] = data[i];
        if(ctx->datalen == 64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    size_t i = ctx->datalen;
    // Pad
    ctx->data[i++] = 0x80;
    if(i > 56){
        while(i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while(i < 56) ctx->data[i++] = 0x00;
    ctx->bitlen += ctx->datalen * 8;
    for(int j = 7; j >= 0; --j){
        ctx->data[56 + (7 - j)] = (uint8_t)((ctx->bitlen >> (j*8)) & 0xff);
    }
    sha256_transform(ctx, ctx->data);
    for(i=0;i<4;i++){
        for(int j=0;j<8;j++){
            hash[i + (j*4)] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xff);
        }
    }
}

static void hmac_sha256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t out[32]){
    uint8_t k_ipad[64] = {0};
    uint8_t k_opad[64] = {0};
    uint8_t tk[32];
    if (key_len > 64) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, key_len);
        sha256_final(&c, tk);
        key = tk;
        key_len = 32;
    }
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);
    for (int i = 0; i < 64; ++i) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }

    uint8_t inner[32];
    SHA256_CTX inner_ctx;
    sha256_init(&inner_ctx);
    sha256_update(&inner_ctx, k_ipad, 64);
    sha256_update(&inner_ctx, data, data_len);
    sha256_final(&inner_ctx, inner);

    SHA256_CTX outer_ctx;
    sha256_init(&outer_ctx);
    sha256_update(&outer_ctx, k_opad, 64);
    sha256_update(&outer_ctx, inner, 32);
    sha256_final(&outer_ctx, out);

    // zeroize
    std::fill(std::begin(k_ipad), std::end(k_ipad), 0);
    std::fill(std::begin(k_opad), std::end(k_opad), 0);
    std::fill(std::begin(tk), std::end(tk), 0);
    std::fill(std::begin(inner), std::end(inner), 0);
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t pass_len,
                               const uint8_t* salt, size_t salt_len,
                               uint32_t iterations,
                               uint8_t* out, size_t out_len){
    uint32_t blocks = (uint32_t)((out_len + 31) / 32);
    uint8_t U[32], T[32];
    std::vector<uint8_t> salt_block(salt_len + 4);
    memcpy(salt_block.data(), salt, salt_len);

    for(uint32_t i = 1; i <= blocks; ++i) {
        salt_block[salt_len] = (uint8_t)((i >> 24) & 0xff);
        salt_block[salt_len+1] = (uint8_t)((i >> 16) & 0xff);
        salt_block[salt_len+2] = (uint8_t)((i >> 8) & 0xff);
        salt_block[salt_len+3] = (uint8_t)(i & 0xff);
        hmac_sha256(password, pass_len, salt_block.data(), salt_len + 4, U);
        memcpy(T, U, 32);
        for(uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, pass_len, U, 32, U);
            for(int k = 0; k < 32; ++k) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i - 1) * 32;
        size_t clen = std::min((size_t)32, out_len - offset);
        memcpy(out + offset, T, clen);
    }
    // zeroize
    memset(U, 0, sizeof(U));
    memset(T, 0, sizeof(T));
    std::fill(salt_block.begin(), salt_block.end(), 0);
}

static bool secure_random_bytes(uint8_t* buf, size_t len){
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t rd = fread(buf, 1, len, f);
    fclose(f);
    return rd == len;
#endif
}

static std::string to_hex(const uint8_t* data, size_t len){
    static const char* hex = "0123456789abcdef";
    std::string out; out.reserve(len*2);
    for(size_t i=0;i<len;i++){
        out.push_back(hex[(data[i] >> 4) & 0xF]);
        out.push_back(hex[data[i] & 0xF]);
    }
    return out;
}

static std::vector<uint8_t> from_hex(const std::string& h){
    if (h.size() % 2 != 0) throw std::invalid_argument("Invalid hex");
    std::vector<uint8_t> out(h.size()/2);
    for(size_t i=0;i<out.size();i++){
        auto hexval = [](char c)->int{
            if(c>='0'&&c<='9') return c-'0';
            if(c>='a'&&c<='f') return c-'a'+10;
            if(c>='A'&&c<='F') return c-'A'+10;
            return -1;
        };
        int hi = hexval(h[2*i]), lo = hexval(h[2*i+1]);
        if(hi<0||lo<0) throw std::invalid_argument("Invalid hex");
        out[i] = (uint8_t)((hi<<4)|lo);
    }
    return out;
}

static bool constant_time_equals(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b){
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for(size_t i=0;i<a.size();i++) r |= (a[i] ^ b[i]);
    return r == 0;
}

class Lse96 {
public:
    static constexpr size_t SALT_LEN = 16;
    static constexpr uint32_t ITERATIONS = 210000;
    static constexpr size_t KEY_LEN = 32;

    static std::unordered_map<std::string, std::string> db;

    static std::vector<uint8_t> generate_salt(size_t len){
        std::vector<uint8_t> s(len);
        if(!secure_random_bytes(s.data(), len)) throw std::runtime_error("RNG failure");
        return s;
    }

    static std::vector<uint8_t> hash_password(const std::string& password, const std::vector<uint8_t>& salt, uint32_t iterations = ITERATIONS, size_t key_len = KEY_LEN){
        std::vector<uint8_t> out(key_len);
        pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                           salt.data(), salt.size(), iterations, out.data(), out.size());
        return out;
    }

    // Register user, returns concatenated hex(salt) + hex(hash)
    static std::string register_user(const std::string& username, const std::string& password){
        if (username.empty()) throw std::invalid_argument("Username required");
        if (password.size() < 8) throw std::invalid_argument("Password too short");
        auto salt = generate_salt(SALT_LEN);
        auto hash = hash_password(password, salt);
        std::string record = to_hex(salt.data(), salt.size()) + to_hex(hash.data(), hash.size());
        db[username] = record;
        // zeroize temp vectors
        std::fill(hash.begin(), hash.end(), 0);
        std::fill(salt.begin(), salt.end(), 0);
        return record;
    }

    static bool verify(const std::string& username, const std::string& password){
        auto it = db.find(username);
        if (it == db.end()) return false;
        const std::string& record = it->second;
        std::string salt_hex = record.substr(0, SALT_LEN * 2);
        std::string hash_hex = record.substr(SALT_LEN * 2);
        auto salt = from_hex(salt_hex);
        auto expected = from_hex(hash_hex);
        auto actual = hash_password(password, salt);
        bool ok = constant_time_equals(expected, actual);
        std::fill(salt.begin(), salt.end(), 0);
        std::fill(expected.begin(), expected.end(), 0);
        std::fill(actual.begin(), actual.end(), 0);
        return ok;
    }

    static std::string get_stored_record(const std::string& username){
        auto it = db.find(username);
        if (it == db.end()) return "";
        return it->second;
    }
};

std::unordered_map<std::string, std::string> Lse96::db;

int main(){
    // 5 test cases
    std::vector<std::string> users = {"alice","bob","charlie","dave","eve"};
    std::vector<std::string> passwords = {"password1!","S33cr3t!","superlongpassword123","Pa$$w0rd123","Secure123!"};

    for(size_t i=0;i<users.size();++i){
        std::string rec = Lse96::register_user(users[i], passwords[i]);
        std::cout << users[i] << " -> " << rec << "\n";
    }

    std::cout << "Verify alice correct: " << (Lse96::verify("alice","password1!") ? "true":"false") << "\n";
    std::cout << "Verify alice wrong: " << (Lse96::verify("alice","wrongpass") ? "true":"false") << "\n";
    std::cout << "Verify bob correct: " << (Lse96::verify("bob","S33cr3t!") ? "true":"false") << "\n";
    std::cout << "Stored for charlie: " << Lse96::get_stored_record("charlie") << "\n";
    std::cout << "Verify eve correct: " << (Lse96::verify("eve","Secure123!") ? "true":"false") << "\n";

    return 0;
}