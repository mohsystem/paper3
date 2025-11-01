#include <bits/stdc++.h>
using namespace std;

// Simple SHA-256 implementation (public domain style)
struct SHA256 {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;

    static inline uint32_t rotr(uint32_t x, uint32_t n){ return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t ep0(uint32_t x){ return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
    static inline uint32_t ep1(uint32_t x){ return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
    static inline uint32_t sig0(uint32_t x){ return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
    static inline uint32_t sig1(uint32_t x){ return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

    static const uint32_t K[64];

    void init() {
        state[0]=0x6a09e667; state[1]=0xbb67ae85; state[2]=0x3c6ef372; state[3]=0xa54ff53a;
        state[4]=0x510e527f; state[5]=0x9b05688c; state[6]=0x1f83d9ab; state[7]=0x5be0cd19;
        bitlen = 0; datalen = 0;
    }

    void transform(const uint8_t data[64]) {
        uint32_t m[64];
        for (int i = 0; i < 16; i++) {
            m[i] = (uint32_t)data[i*4] << 24 | (uint32_t)data[i*4+1] << 16 | (uint32_t)data[i*4+2] << 8 | (uint32_t)data[i*4+3];
        }
        for (int i = 16; i < 64; i++) m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
        uint32_t a=state[0],b=state[1],c=state[2],d=state[3],e=state[4],f=state[5],g=state[6],h=state[7];
        for (int i = 0; i < 64; i++) {
            uint32_t t1 = h + ep1(e) + ch(e,f,g) + K[i] + m[i];
            uint32_t t2 = ep0(a) + maj(a,b,c);
            h=g; g=f; f=e; e=d + t1; d=c; c=b; b=a; a=t1 + t2;
        }
        state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d; state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
    }

    void update(const uint8_t* data_in, size_t len) {
        for (size_t i = 0; i < len; i++) {
            data[datalen++] = data_in[i];
            if (datalen == 64) {
                transform(data);
                bitlen += 512;
                datalen = 0;
            }
        }
    }

    void final(uint8_t hash[32]) {
        size_t i = datalen;
        // Pad
        data[i++] = 0x80;
        if (i > 56) {
            while (i < 64) data[i++] = 0x00;
            transform(data);
            i = 0;
        }
        while (i < 56) data[i++] = 0x00;
        bitlen += datalen * 8ULL;
        for (int j = 7; j >= 0; --j) data[i++] = (uint8_t)((bitlen >> (j*8)) & 0xFF);
        transform(data);
        for (int j = 0; j < 8; j++) {
            hash[j*4]   = (uint8_t)((state[j] >> 24) & 0xFF);
            hash[j*4+1] = (uint8_t)((state[j] >> 16) & 0xFF);
            hash[j*4+2] = (uint8_t)((state[j] >> 8) & 0xFF);
            hash[j*4+3] = (uint8_t)(state[j] & 0xFF);
        }
    }
};

const uint32_t SHA256::K[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_bytes(const vector<uint8_t>& data, uint8_t out[32]) {
    SHA256 ctx; ctx.init(); ctx.update(data.data(), data.size()); ctx.final(out);
}

// Derive salt deterministically from username (for demo; real systems should use random salt)
static vector<uint8_t> derive_salt_from_username(const string& username) {
    string s = string("salt:") + username;
    vector<uint8_t> in(s.begin(), s.end());
    uint8_t h[32]; sha256_bytes(in, h);
    return vector<uint8_t>(h, h + 16); // 16-byte salt
}

// Simple iterative salted SHA-256 KDF (for demo; use PBKDF2/bcrypt/scrypt/Argon2 in production)
static void kdf_iter_sha256(const string& password, const vector<uint8_t>& salt, uint32_t iterations, uint8_t out[32]) {
    vector<uint8_t> initial;
    initial.reserve(salt.size() + password.size());
    initial.insert(initial.end(), salt.begin(), salt.end());
    initial.insert(initial.end(), password.begin(), password.end());
    sha256_bytes(initial, out);
    vector<uint8_t> tmp(out, out + 32);
    for (uint32_t i = 1; i < iterations; ++i) {
        sha256_bytes(tmp, out);
        tmp.assign(out, out + 32);
    }
}

// Constant-time compare
static bool ct_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t acc = 0;
    for (size_t i = 0; i < n; ++i) acc |= (uint8_t)(a[i] ^ b[i]);
    return acc == 0;
}

struct UserRecord {
    vector<uint8_t> salt;
    array<uint8_t, 32> hash;
    uint32_t iterations;
};

class AuthDB {
    unordered_map<string, UserRecord> db;
    uint32_t iterations = 100000;
public:
    void add_user(const string& username, const string& password) {
        if (username.empty()) return;
        vector<uint8_t> salt = derive_salt_from_username(username);
        uint8_t out[32];
        kdf_iter_sha256(password, salt, iterations, out);
        UserRecord rec;
        rec.salt = salt;
        copy(out, out + 32, rec.hash.begin());
        rec.iterations = iterations;
        db[username] = rec;
    }

    // Returns "/home" on success, otherwise empty string
    string login(const string& username, const string& password) {
        auto it = db.find(username);
        if (it == db.end()) return "";
        uint8_t out[32];
        kdf_iter_sha256(password, it->second.salt, it->second.iterations, out);
        if (ct_equal(out, it->second.hash.data(), 32)) return "/home";
        return "";
    }
};

int main() {
    AuthDB auth;
    auth.add_user("alice", "Password123!");
    auth.add_user("bob", "s3cur3");

    vector<pair<string,string>> tests = {
        {"alice", "Password123!"},
        {"alice", "wrong"},
        {"bob", "s3cur3"},
        {"charlie", "anything"},
        {"", ""}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        string res = auth.login(tests[i].first, tests[i].second);
        cout << "Test " << (i+1) << ": " << (res.empty() ? "login failed" : res) << "\n";
    }
    return 0;
}