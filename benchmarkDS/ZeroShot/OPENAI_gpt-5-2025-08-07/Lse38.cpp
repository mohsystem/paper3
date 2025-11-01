#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256 implementation (public domain style)
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static const uint32_t k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static inline uint32_t ep1(uint32_t x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static inline uint32_t sig0(uint32_t x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i=0;i<16;i++) {
        m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 | (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++) m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];

    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];

    for (int i=0;i<64;i++) {
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + k[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx){
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len){
    for (size_t i=0;i<len;i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[32]){
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i=0;i<4;i++) {
        for (int j=0;j<8;j++) {
            hash[i + j*4] = (ctx->state[j] >> (24 - i*8)) & 0xff;
        }
    }
}

static vector<uint8_t> sha256(const vector<uint8_t>& data){
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data.data(), data.size());
    vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

static bool ct_equal(const vector<uint8_t>& a, const vector<uint8_t>& b){
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i=0;i<a.size();++i) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

struct UserRec {
    vector<uint8_t> salt;
    vector<uint8_t> hash;
};

class Auth {
    unordered_map<string, UserRec> db;
    static vector<uint8_t> derive(const string& password, const vector<uint8_t>& salt, int iterations){
        vector<uint8_t> pwd(password.begin(), password.end());
        vector<uint8_t> buf;
        buf.reserve(salt.size() + pwd.size() + 32);
        // Initial hash: H(salt || password)
        buf = salt;
        buf.insert(buf.end(), pwd.begin(), pwd.end());
        vector<uint8_t> h = sha256(buf);
        for (int i=1;i<iterations;i++) {
            vector<uint8_t> mix;
            mix.reserve(h.size() + salt.size() + pwd.size());
            mix.insert(mix.end(), h.begin(), h.end());
            mix.insert(mix.end(), salt.begin(), salt.end());
            mix.insert(mix.end(), pwd.begin(), pwd.end());
            h = sha256(mix);
        }
        // zeroize pwd vector
        for (auto &c : pwd) c = 0;
        return h;
    }
    static vector<uint8_t> random_bytes(size_t n){
        vector<uint8_t> out(n);
        std::random_device rd;
        for (size_t i=0;i<n;i++) out[i] = static_cast<uint8_t>(rd());
        return out;
    }
public:
    bool add_user(const string& username, const string& password){
        if (username.empty()) return false;
        if (db.find(username) != db.end()) return false;
        auto salt = random_bytes(16);
        auto hash = derive(password, salt, 50000);
        db[username] = UserRec{salt, hash};
        return true;
    }
    string login(const string& username, const string& password){
        auto it = db.find(username);
        if (it == db.end()) return "/login";
        auto computed = derive(password, it->second.salt, 50000);
        return ct_equal(computed, it->second.hash) ? "/dashboard" : "/login";
    }
};

int main(){
    Auth auth;
    auth.add_user("alice", "Correct$Horse1");
    auth.add_user("bob", "S3curePass!");

    cout << auth.login("alice", "Correct$Horse1") << "\n"; // /dashboard
    cout << auth.login("alice", "wrongpass") << "\n";      // /login
    cout << auth.login("unknown", "whatever") << "\n";     // /login
    cout << auth.login("", "") << "\n";                    // /login
    cout << auth.login("bob", "S3curePass!") << "\n";      // /dashboard
    return 0;
}