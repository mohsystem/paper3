#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <algorithm>

// Minimal secure login system with email change, using PBKDF2-HMAC-SHA256 (self-contained)

namespace crypto {

// SHA-256 implementation (public domain style)
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

static inline uint32_t ROTR(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTR(x,7) ^ ROTR(x,18) ^ (x >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTR(x,17) ^ ROTR(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX* ctx, const uint8_t data[64]){
    uint32_t m[64];
    for (int i=0;i<16;i++){
        m[i] = (uint32_t)data[i*4]<<24 | (uint32_t)data[i*4+1]<<16 | (uint32_t)data[i*4+2]<<8 | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++){
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++){
        uint32_t t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        uint32_t t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
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
        if (ctx->datalen==64){
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX* ctx, uint8_t hash[32]){
    uint32_t i = ctx->datalen;
    // Pad
    ctx->data[i++] = 0x80;
    if (i > 56){
        while(i<64) ctx->data[i++]=0x00;
        sha256_transform(ctx, ctx->data);
        i=0;
    }
    while(i<56) ctx->data[i++]=0x00;
    ctx->bitlen += ctx->datalen * 8ull;
    // Append length in bits (big endian)
    for (int j=7;j>=0;j--){
        ctx->data[56 + (7-j)] = (uint8_t)((ctx->bitlen >> (j*8)) & 0xffu);
    }
    sha256_transform(ctx, ctx->data);
    for (int j=0;j<8;j++){
        hash[j*4  ] = (uint8_t)((ctx->state[j] >> 24) & 0xffu);
        hash[j*4+1] = (uint8_t)((ctx->state[j] >> 16) & 0xffu);
        hash[j*4+2] = (uint8_t)((ctx->state[j] >> 8) & 0xffu);
        hash[j*4+3] = (uint8_t)(ctx->state[j] & 0xffu);
    }
    // Wipe context
    std::memset(ctx, 0, sizeof(*ctx));
}

static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]){
    const size_t block = 64;
    uint8_t k0[64];
    if (keylen > block) {
        // hash key
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, keylen);
        sha256_final(&c, k0);
        std::memset(k0+32, 0, 32);
    } else {
        std::memset(k0, 0, sizeof(k0));
        std::memcpy(k0, key, keylen);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i=0;i<64;i++){ ipad[i] = k0[i] ^ 0x36u; opad[i] = k0[i] ^ 0x5cu; }
    // inner
    SHA256_CTX ci;
    sha256_init(&ci);
    sha256_update(&ci, ipad, 64);
    sha256_update(&ci, data, datalen);
    uint8_t inner[32];
    sha256_final(&ci, inner);
    // outer
    SHA256_CTX co;
    sha256_init(&co);
    sha256_update(&co, opad, 64);
    sha256_update(&co, inner, 32);
    sha256_final(&co, out);
    // wipe
    std::memset(k0, 0, sizeof(k0));
    std::memset(ipad, 0, sizeof(ipad));
    std::memset(opad, 0, sizeof(opad));
    std::memset(inner, 0, sizeof(inner));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* dk, size_t dkLen){
    size_t hLen = 32;
    uint32_t blocks = static_cast<uint32_t>((dkLen + hLen - 1) / hLen);
    std::vector<uint8_t> U(hLen);
    std::vector<uint8_t> T(hLen);
    std::vector<uint8_t> saltBlock(saltlen + 4);
    std::memcpy(saltBlock.data(), salt, saltlen);

    for (uint32_t i = 1; i <= blocks; i++) {
        saltBlock[saltlen]   = (uint8_t)((i >> 24) & 0xffu);
        saltBlock[saltlen+1] = (uint8_t)((i >> 16) & 0xffu);
        saltBlock[saltlen+2] = (uint8_t)((i >> 8) & 0xffu);
        saltBlock[saltlen+3] = (uint8_t)(i & 0xffu);

        hmac_sha256(password, passlen, saltBlock.data(), saltlen + 4, U.data());
        std::memcpy(T.data(), U.data(), hLen);

        for (uint32_t j = 2; j <= iterations; j++) {
            hmac_sha256(password, passlen, U.data(), hLen, U.data());
            for (size_t k = 0; k < hLen; k++) {
                T[k] ^= U[k];
            }
        }

        size_t offset = (size_t)(i - 1) * hLen;
        size_t cp = std::min(hLen, dkLen - offset);
        std::memcpy(dk + offset, T.data(), cp);
    }

    // Wipe sensitive
    std::fill(U.begin(), U.end(), 0);
    std::fill(T.begin(), T.end(), 0);
    std::fill(saltBlock.begin(), saltBlock.end(), 0);
}

} // namespace crypto

static bool constant_time_eq(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b){
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i=0;i<a.size();i++) r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}

static bool get_secure_random_bytes(uint8_t* out, size_t len){
    // Try /dev/urandom
    std::ifstream ur("/dev/urandom", std::ios::in | std::ios::binary);
    if (ur.good()){
        ur.read(reinterpret_cast<char*>(out), (std::streamsize)len);
        if ((size_t)ur.gcount() == len) return true;
    }
    // Fallback to std::random_device
    std::random_device rd;
    for (size_t i=0;i<len;i++){
        out[i] = (uint8_t)(rd() & 0xFFu);
    }
    return true;
}

static void secure_zero(void* v, size_t n){
    volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(v);
    while (n--) { *p++ = 0; }
}

static bool is_password_strong(const std::string& pw){
    if (pw.size() < 12) return false;
    bool u=false,l=false,d=false,s=false;
    for(char c: pw){
        if (c >= 'A' && c <= 'Z') u=true;
        else if (c >= 'a' && c <= 'z') l=true;
        else if (c >= '0' && c <= '9') d=true;
        else s=true;
    }
    return u&&l&&d&&s;
}

static bool is_email_valid(const std::string& email){
    if (email.size() < 5 || email.size() > 254) return false;
    auto at = email.find('@');
    if (at == std::string::npos || at == 0 || at == email.size()-1) return false;
    auto dot = email.find('.', at+1);
    if (dot == std::string::npos || dot == email.size()-1) return false;
    return true;
}

struct User {
    std::string username;
    std::string email;
    std::vector<uint8_t> salt;    // 16 bytes
    std::vector<uint8_t> hash;    // 32 bytes
    bool loggedIn;
    User(const std::string& u, const std::string& e): username(u), email(e), loggedIn(false) {}
};

static bool set_password(User& user, const std::string& password){
    if (!is_password_strong(password)) return false;
    user.salt.assign(16, 0);
    if (!get_secure_random_bytes(user.salt.data(), user.salt.size())) return false;
    user.hash.assign(32, 0);
    crypto::pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                               user.salt.data(), user.salt.size(),
                               210000, user.hash.data(), user.hash.size());
    return true;
}

static bool verify_password(const User& user, const std::string& password){
    if (user.salt.size()!=16 || user.hash.size()!=32) return false;
    std::vector<uint8_t> derived(32, 0);
    crypto::pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                               user.salt.data(), user.salt.size(),
                               210000, derived.data(), derived.size());
    bool ok = constant_time_eq(user.hash, derived);
    secure_zero(derived.data(), derived.size());
    return ok;
}

static bool login(User& user, const std::string& username, const std::string& password){
    if (username != user.username) return false;
    if (verify_password(user, password)) {
        user.loggedIn = true;
        return true;
    }
    return false;
}

static bool change_email(User& user, const std::string& old_email, const std::string& new_email, const std::string& confirm_password){
    if (!user.loggedIn) return false;
    if (old_email != user.email) return false;
    if (!is_email_valid(new_email)) return false;
    if (!verify_password(user, confirm_password)) return false;
    user.email = new_email;
    return true;
}

static std::string generate_strong_password(size_t length = 16){
    const std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string lower = "abcdefghijklmnopqrstuvwxyz";
    const std::string digits = "0123456789";
    const std::string special = "!@#$%^&*()-_=+[]{};:,.?/|";
    const std::string all = upper + lower + digits + special;
    if (length < 12) length = 12;

    std::string pw;
    pw.push_back(upper[std::rand()%upper.size()]);
    pw.push_back(lower[std::rand()%lower.size()]);
    pw.push_back(digits[std::rand()%digits.size()]);
    pw.push_back(special[std::rand()%special.size()]);

    std::vector<uint8_t> rb(length - 4);
    get_secure_random_bytes(rb.data(), rb.size());
    for (size_t i=0;i<rb.size();i++){
        pw.push_back(all[rb[i] % all.size()]);
    }
    // shuffle
    for (size_t i = pw.size() - 1; i > 0; i--) {
        uint8_t b;
        get_secure_random_bytes(&b, 1);
        size_t j = b % (i + 1);
        std::swap(pw[i], pw[j]);
    }
    return pw;
}

int main(){
    User user("alice", "alice@example.com");
    std::string password = generate_strong_password(16);
    if (!set_password(user, password)) {
        std::cout << "Setup failed\n";
        return 0;
    }

    // Test 1: Attempt change while not logged in -> false
    bool t1 = change_email(user, "alice@example.com", "newalice@example.com", password);
    std::cout << "Test1 change while not logged in: " << (t1 ? "true" : "false") << "\n";

    // Test 2: Login with correct password -> true
    bool t2 = login(user, "alice", password);
    std::cout << "Test2 login with correct password: " << (t2 ? "true" : "false") << "\n";

    // Test 3: Change with wrong old email -> false
    bool t3 = change_email(user, "wrong@example.com", "newalice@example.com", password);
    std::cout << "Test3 change with wrong old email: " << (t3 ? "true" : "false") << "\n";

    // Test 4: Change with wrong password -> false
    bool t4 = change_email(user, "alice@example.com", "newalice@example.com", std::string("WrongPass!234"));
    std::cout << "Test4 change with wrong password: " << (t4 ? "true" : "false") << "\n";

    // Test 5: Change with correct old email and password -> true
    bool t5 = change_email(user, "alice@example.com", "newalice@example.com", password);
    std::cout << "Test5 change with correct old email and password: " << (t5 ? "true" : "false") << "\n";

    // Zeroize password in memory
    secure_zero(&password[0], password.size());
    return 0;
}