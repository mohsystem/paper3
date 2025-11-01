#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256 implementation with safe patterns.

// ---- SHA-256 ----
namespace sha256 {
    static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t bsig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline uint32_t bsig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline uint32_t ssig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline uint32_t ssig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    static const uint32_t K[64] = {
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
    };

    struct Ctx {
        uint64_t bitlen;
        uint32_t state[8];
        uint8_t buffer[64];
        size_t buffer_len;
    };

    static void init(Ctx &ctx) {
        ctx.bitlen = 0;
        ctx.state[0]=0x6a09e667u; ctx.state[1]=0xbb67ae85u; ctx.state[2]=0x3c6ef372u; ctx.state[3]=0xa54ff53au;
        ctx.state[4]=0x510e527fu; ctx.state[5]=0x9b05688cu; ctx.state[6]=0x1f83d9abu; ctx.state[7]=0x5be0cd19u;
        ctx.buffer_len = 0;
    }

    static void transform(Ctx &ctx, const uint8_t data[64]) {
        uint32_t w[64];
        for (int i=0;i<16;i++) {
            w[i] = (uint32_t)data[i*4+0]<<8*3 | (uint32_t)data[i*4+1]<<8*2 | (uint32_t)data[i*4+2]<<8*1 | (uint32_t)data[i*4+3];
        }
        for (int i=16;i<64;i++) w[i] = ssig1(w[i-2]) + w[i-7] + ssig0(w[i-15]) + w[i-16];
        uint32_t a=ctx.state[0],b=ctx.state[1],c=ctx.state[2],d=ctx.state[3],e=ctx.state[4],f=ctx.state[5],g=ctx.state[6],h=ctx.state[7];
        for (int i=0;i<64;i++) {
            uint32_t t1 = h + bsig1(e) + ch(e,f,g) + K[i] + w[i];
            uint32_t t2 = bsig0(a) + maj(a,b,c);
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        ctx.state[0]+=a; ctx.state[1]+=b; ctx.state[2]+=c; ctx.state[3]+=d;
        ctx.state[4]+=e; ctx.state[5]+=f; ctx.state[6]+=g; ctx.state[7]+=h;
    }

    static void update(Ctx &ctx, const uint8_t *data, size_t len) {
        while (len > 0) {
            size_t to_copy = min(len, 64 - ctx.buffer_len);
            memcpy(ctx.buffer + ctx.buffer_len, data, to_copy);
            ctx.buffer_len += to_copy;
            data += to_copy;
            len -= to_copy;
            if (ctx.buffer_len == 64) {
                transform(ctx, ctx.buffer);
                ctx.bitlen += 512;
                ctx.buffer_len = 0;
            }
        }
    }

    static void final(Ctx &ctx, uint8_t out[32]) {
        ctx.bitlen += ctx.buffer_len * 8;
        // append '1' bit
        ctx.buffer[ctx.buffer_len++] = 0x80;
        if (ctx.buffer_len > 56) {
            while (ctx.buffer_len < 64) ctx.buffer[ctx.buffer_len++] = 0x00;
            transform(ctx, ctx.buffer);
            ctx.buffer_len = 0;
        }
        while (ctx.buffer_len < 56) ctx.buffer[ctx.buffer_len++] = 0x00;
        // append length (big endian)
        for (int i=7;i>=0;i--) {
            ctx.buffer[ctx.buffer_len++] = (uint8_t)((ctx.bitlen >> (i*8)) & 0xFFu);
        }
        transform(ctx, ctx.buffer);
        for (int i=0;i<8;i++) {
            out[i*4+0] = (uint8_t)((ctx.state[i] >> 24) & 0xFFu);
            out[i*4+1] = (uint8_t)((ctx.state[i] >> 16) & 0xFFu);
            out[i*4+2] = (uint8_t)((ctx.state[i] >> 8) & 0xFFu);
            out[i*4+3] = (uint8_t)(ctx.state[i] & 0xFFu);
        }
    }

    static vector<uint8_t> hash(const vector<uint8_t>& data) {
        Ctx ctx; init(ctx);
        update(ctx, data.data(), data.size());
        vector<uint8_t> out(32);
        final(ctx, out.data());
        return out;
    }
}

// HMAC-SHA256
static vector<uint8_t> hmac_sha256(const vector<uint8_t>& key, const uint8_t* data, size_t len) {
    const size_t blockSize = 64;
    vector<uint8_t> k(blockSize, 0x00);
    if (key.size() > blockSize) {
        vector<uint8_t> kh = sha256::hash(key);
        memcpy(k.data(), kh.data(), kh.size());
    } else {
        memcpy(k.data(), key.data(), key.size());
    }
    vector<uint8_t> ipad(blockSize, 0x36), opad(blockSize, 0x5c);
    for (size_t i=0;i<blockSize;i++) {
        ipad[i] ^= k[i];
        opad[i] ^= k[i];
    }
    sha256::Ctx ictx; sha256::init(ictx);
    sha256::update(ictx, ipad.data(), ipad.size());
    sha256::update(ictx, data, len);
    uint8_t inner[32]; sha256::final(ictx, inner);

    sha256::Ctx octx; sha256::init(octx);
    sha256::update(octx, opad.data(), opad.size());
    sha256::update(octx, inner, sizeof(inner));
    vector<uint8_t> out(32);
    sha256::final(octx, out.data());
    // wipe sensitive
    volatile uint8_t wipe = 0;
    for (auto &b : k) wipe ^= b, b = 0;
    for (auto &b : ipad) wipe ^= b, b = 0;
    for (auto &b : opad) wipe ^= b, b = 0;
    (void)wipe;
    memset(inner, 0, sizeof(inner));
    return out;
}

// PBKDF2-HMAC-SHA256
static vector<uint8_t> pbkdf2_hmac_sha256(const string& password, const vector<uint8_t>& salt, uint32_t iterations, size_t dkLen) {
    vector<uint8_t> dk(dkLen, 0);
    vector<uint8_t> key(password.begin(), password.end());
    uint32_t blockCount = (uint32_t)((dkLen + 31) / 32);
    size_t pos = 0;
    for (uint32_t i = 1; i <= blockCount; i++) {
        // U1 = HMAC(P, S || INT(i))
        vector<uint8_t> s(salt);
        uint8_t be[4] = { (uint8_t)(i>>24), (uint8_t)(i>>16), (uint8_t)(i>>8), (uint8_t)(i) };
        s.insert(s.end(), be, be+4);
        vector<uint8_t> u = hmac_sha256(key, s.data(), s.size());
        vector<uint8_t> t = u;
        for (uint32_t j = 2; j <= iterations; j++) {
            u = hmac_sha256(key, u.data(), u.size());
            for (size_t k = 0; k < t.size(); k++) t[k] ^= u[k];
        }
        size_t to_copy = min((size_t)32, dkLen - pos);
        memcpy(dk.data() + pos, t.data(), to_copy);
        pos += to_copy;
        // wipe sensitives in loop
        volatile uint8_t wipe = 0;
        for (auto &b : s) { wipe ^= b; b = 0; }
        for (auto &b : u) { wipe ^= b; b = 0; }
        for (auto &b : t) { wipe ^= b; b = 0; }
        (void)wipe;
    }
    // wipe key
    volatile uint8_t wipe = 0;
    for (auto &b : key) { wipe ^= b; b = 0; }
    (void)wipe;
    return dk;
}

static bool constantTimeEqual(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i = 0; i < a.size(); i++) r |= (a[i] ^ b[i]);
    return r == 0;
}

static bool getRandomBytes(uint8_t* out, size_t len) {
    // Use /dev/urandom
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t total = 0;
    while (total < len) {
        size_t n = fread(out + total, 1, len - total, f);
        if (n == 0) { fclose(f); return false; }
        total += n;
    }
    fclose(f);
    return true;
}

struct UserRecord {
    vector<uint8_t> salt;
    vector<uint8_t> hash;
    uint32_t iterations;
};

class SecureAuthDB {
public:
    bool registerUser(const string& username, const string& password) {
        if (!validateUsername(username)) return false;
        if (!passwordPolicy(password)) return false;
        if (db.find(username) != db.end()) return false;
        vector<uint8_t> salt(16);
        if (!getRandomBytes(salt.data(), salt.size())) return false;
        vector<uint8_t> hash = pbkdf2_hmac_sha256(password, salt, ITERATIONS, 32);
        db[username] = UserRecord{salt, hash, ITERATIONS};
        return true;
    }

    bool authenticate(const string& username, const string& password) const {
        if (!validateUsername(username)) return false;
        auto it = db.find(username);
        if (it == db.end()) return false;
        const UserRecord& rec = it->second;
        vector<uint8_t> computed = pbkdf2_hmac_sha256(password, rec.salt, rec.iterations, rec.hash.size());
        bool eq = constantTimeEqual(rec.hash, computed);
        // wipe
        volatile uint8_t wipe = 0;
        for (auto &b : computed) { wipe ^= b; b = 0; }
        (void)wipe;
        return eq;
    }

    string login(const string& username, const string& password) const {
        return authenticate(username, password) ? "Redirect: /dashboard" : "Redirect: /login";
    }

private:
    unordered_map<string, UserRecord> db;
    static constexpr uint32_t ITERATIONS = 210000;

    static bool validateUsername(const string& u) {
        if (u.size() < 3 || u.size() > 32) return false;
        for (char c : u) {
            if (!(isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
        }
        return true;
    }

    static bool passwordPolicy(const string& p) {
        if (p.size() < 10 || p.size() > 128) return false;
        bool up=false, lo=false, di=false, sp=false;
        for (char c : p) {
            if (isupper(static_cast<unsigned char>(c))) up = true;
            else if (islower(static_cast<unsigned char>(c))) lo = true;
            else if (isdigit(static_cast<unsigned char>(c))) di = true;
            else sp = true;
        }
        return up && lo && di && sp;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    SecureAuthDB db;
    cout << "Register alice: " << (db.registerUser("alice", "Str0ng!Pass1") ? "true" : "false") << "\n";
    cout << "Register bob: " << (db.registerUser("bob", "B0b$Secure#2024") ? "true" : "false") << "\n";
    cout << "Register charlie: " << (db.registerUser("charlie", "Ch@rlie2025!!") ? "true" : "false") << "\n";

    vector<pair<string,string>> tests = {
        {"alice", "Str0ng!Pass1"},
        {"alice", "wrongpassword!"},
        {"unknown", "AnyPassw0rd!"},
        {"bad user", "Xx1!invalid"},
        {"bob", "B0b$Secure#2024"}
    };
    for (size_t i=0;i<tests.size();i++) {
        auto &t = tests[i];
        cout << "Test " << (i+1) << " login(" << t.first << ", [redacted]): " << db.login(t.first, t.second) << "\n";
    }
    return 0;
}