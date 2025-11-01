#include <bits/stdc++.h>
using namespace std;

// Secure login with PBKDF2-HMAC-SHA256, per-user random salts, and input validation.
// Single-file C++ implementation without external libraries.

// ---------- SHA-256 Implementation ----------
struct SHA256 {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;

    static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t ep0(uint32_t x) { return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
    static inline uint32_t ep1(uint32_t x) { return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
    static inline uint32_t sig0(uint32_t x) { return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
    static inline uint32_t sig1(uint32_t x) { return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

    static const uint32_t K[64];

    void init() {
        state[0]=0x6a09e667; state[1]=0xbb67ae85; state[2]=0x3c6ef372; state[3]=0xa54ff53a;
        state[4]=0x510e527f; state[5]=0x9b05688c; state[6]=0x1f83d9ab; state[7]=0x5be0cd19;
        bitlen = 0; datalen = 0;
        memset(data, 0, sizeof(data));
    }

    static inline uint32_t be32(const uint8_t b[4]) {
        return (uint32_t)b[0]<<24 | (uint32_t)b[1]<<16 | (uint32_t)b[2]<<8 | (uint32_t)b[3];
    }

    void transform(const uint8_t block[64]) {
        uint32_t m[64];
        for (int i=0;i<16;i++) {
            m[i] = be32(block + i*4);
        }
        for (int i=16;i<64;i++) {
            m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
        }
        uint32_t a=state[0], b=state[1], c=state[2], d=state[3], e=state[4], f=state[5], g=state[6], h=state[7];
        for (int i=0;i<64;i++) {
            uint32_t t1 = h + ep1(e) + ch(e,f,g) + K[i] + m[i];
            uint32_t t2 = ep0(a) + maj(a,b,c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
        state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
    }

    void update(const uint8_t* in, size_t len) {
        for (size_t i=0;i<len;i++) {
            data[datalen++] = in[i];
            if (datalen == 64) {
                transform(data);
                bitlen += 512;
                datalen = 0;
            }
        }
    }

    void final(uint8_t out[32]) {
        uint64_t bits = bitlen + datalen*8;
        data[datalen++] = 0x80;
        if (datalen > 56) {
            while (datalen < 64) data[datalen++] = 0;
            transform(data);
            datalen = 0;
        }
        while (datalen < 56) data[datalen++] = 0;
        uint8_t lenbe[8];
        for (int i=7;i>=0;i--) { lenbe[i] = (uint8_t)(bits & 0xFF); bits >>= 8; }
        for (int i=0;i<8;i++) data[56+i] = lenbe[i];
        transform(data);
        for (int i=0;i<8;i++) {
            out[i*4+0] = (uint8_t)((state[i] >> 24) & 0xFF);
            out[i*4+1] = (uint8_t)((state[i] >> 16) & 0xFF);
            out[i*4+2] = (uint8_t)((state[i] >> 8) & 0xFF);
            out[i*4+3] = (uint8_t)(state[i] & 0xFF);
        }
        // wipe
        memset(data, 0, sizeof(data));
        memset(state, 0, sizeof(state));
        datalen = 0; bitlen = 0;
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

// ---------- HMAC-SHA256 ----------
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* msg, size_t msglen, uint8_t out[32]) {
    uint8_t kopad[64], kipad[64], khash[32];
    if (keylen > 64) {
        SHA256 ctx;
        ctx.init(); ctx.update(key, keylen); ctx.final(khash);
        key = khash; keylen = 32;
    }
    memset(kopad, 0, sizeof(kopad));
    memset(kipad, 0, sizeof(kipad));
    memcpy(kopad, key, keylen);
    memcpy(kipad, key, keylen);
    for (int i=0;i<64;i++) {
        kipad[i] ^= 0x36;
        kopad[i] ^= 0x5c;
    }
    uint8_t inner[32];
    SHA256 ictx; ictx.init(); ictx.update(kipad, 64); ictx.update(msg, msglen); ictx.final(inner);
    SHA256 octx; octx.init(); octx.update(kopad, 64); octx.update(inner, 32); octx.final(out);
    // wipe
    memset(kopad,0,sizeof(kopad)); memset(kipad,0,sizeof(kipad));
    memset(khash,0,sizeof(khash)); memset(inner,0,sizeof(inner));
}

// ---------- PBKDF2-HMAC-SHA256 ----------
static bool pbkdf2_hmac_sha256(const uint8_t* password, size_t pwlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iterations,
                               uint8_t* out, size_t dkLen) {
    if (iterations < 1 || dkLen == 0) return false;
    size_t hLen = 32;
    uint32_t blocks = (uint32_t)((dkLen + hLen - 1) / hLen);
    vector<uint8_t> U(hLen), T(hLen), saltBlock(saltlen + 4);
    memcpy(saltBlock.data(), salt, saltlen);
    for (uint32_t i = 1; i <= blocks; i++) {
        saltBlock[saltlen+0] = (uint8_t)((i >> 24) & 0xFF);
        saltBlock[saltlen+1] = (uint8_t)((i >> 16) & 0xFF);
        saltBlock[saltlen+2] = (uint8_t)((i >> 8) & 0xFF);
        saltBlock[saltlen+3] = (uint8_t)(i & 0xFF);
        hmac_sha256(password, pwlen, saltBlock.data(), saltBlock.size(), U.data());
        memcpy(T.data(), U.data(), hLen);
        for (uint32_t j=2; j<=iterations; j++) {
            hmac_sha256(password, pwlen, U.data(), hLen, U.data());
            for (size_t k=0;k<hLen;k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i - 1) * hLen;
        size_t cp = min(hLen, dkLen - offset);
        memcpy(out + offset, T.data(), cp);
    }
    // wipe
    fill(U.begin(), U.end(), 0);
    fill(T.begin(), T.end(), 0);
    fill(saltBlock.begin(), saltBlock.end(), 0);
    return true;
}

// ---------- CSPRNG ----------
static bool get_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    // Use BCryptGenRandom if available
    HCRYPTPROV prov;
    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) return false;
    BOOL ok = CryptGenRandom(prov, (DWORD)len, (BYTE*)buf);
    CryptReleaseContext(prov, 0);
    return ok == TRUE;
#else
    // POSIX: read from /dev/urandom
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t off = 0;
    while (off < len) {
        size_t r = fread(buf + off, 1, len - off, f);
        if (r == 0) { fclose(f); return false; }
        off += r;
    }
    fclose(f);
    return true;
#endif
}

static bool constant_time_eq(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t diff = 0;
    for (size_t i=0;i<n;i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

struct User {
    string username;
    vector<uint8_t> salt;
    vector<uint8_t> hash;
    uint32_t iterations;
};

struct InMemoryDB {
    unordered_map<string, User> users;

    bool addUser(const string& username, const string& password) {
        if (!validateUsername(username) || !validatePassword(password)) return false;
        if (users.find(username) != users.end()) return false;
        const size_t SALT_LEN = 16, HASH_LEN = 32;
        vector<uint8_t> salt(SALT_LEN), hash(HASH_LEN);
        if (!get_random_bytes(salt.data(), SALT_LEN)) return false;
        if (!pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                                salt.data(), salt.size(), 210000, hash.data(), hash.size())) return false;
        users.emplace(username, User{username, salt, hash, 210000});
        return true;
    }

    const User* findByUsername(const string& username) const {
        auto it = users.find(username);
        if (it == users.end()) return nullptr;
        return &it->second;
    }
};

static bool validateUsername(const string& username) {
    if (username.size() < 3 || username.size() > 32) return false;
    for (char c : username) {
        if (!(isalnum((unsigned char)c) || c=='.' || c=='_' || c=='-')) return false;
    }
    return true;
}

static bool validatePassword(const string& password) {
    return password.size() >= 8 && password.size() <= 128;
}

string handleLogin(InMemoryDB& db, const string& username, const string& password) {
    if (!validateUsername(username) || !validatePassword(password)) {
        return "REDIRECT:/login";
    }
    const User* u = db.findByUsername(username);
    if (!u) return "REDIRECT:/login";
    vector<uint8_t> computed(u->hash.size());
    if (!pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                            u->salt.data(), u->salt.size(), u->iterations, computed.data(), computed.size())) {
        return "REDIRECT:/login";
    }
    bool ok = constant_time_eq(computed.data(), u->hash.data(), u->hash.size());
    // wipe computed
    fill(computed.begin(), computed.end(), 0);
    return ok ? "REDIRECT:/dashboard" : "REDIRECT:/login";
}

int main() {
    InMemoryDB db;
    // Create test users
    bool a = db.addUser("alice", "CorrectHorse1!");
    bool b = db.addUser("bob", "S3cureP@ssw0rd");
    if (!(a && b)) {
        cout << "Initialization failed\n";
        return 1;
    }

    vector<pair<string,string>> tests = {
        {"alice", "CorrectHorse1!"},
        {"alice", "wrongpass"},
        {"charlie", "NoSuchUser1!"},
        {"x", "SomeValid1!"},
        {"bob", "short"}
    };

    for (size_t i=0;i<tests.size();i++) {
        cout << "Test " << (i+1) << ": " << handleLogin(db, tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}