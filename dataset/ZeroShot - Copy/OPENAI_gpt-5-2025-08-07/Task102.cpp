#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256 implementation
struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t ep0(uint32_t x) { return rotr(x,2) ^ rotr(x,13) ^ rotr(x,22); }
static inline uint32_t ep1(uint32_t x) { return rotr(x,6) ^ rotr(x,11) ^ rotr(x,25); }
static inline uint32_t sig0(uint32_t x) { return rotr(x,7) ^ rotr(x,18) ^ (x >> 3); }
static inline uint32_t sig1(uint32_t x) { return rotr(x,17) ^ rotr(x,19) ^ (x >> 10); }

static const uint32_t k256[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) {
        m[i] = (uint32_t(data[i*4]) << 24) | (uint32_t(data[i*4+1]) << 16) | (uint32_t(data[i*4+2]) << 8) | (uint32_t(data[i*4+3]));
    }
    for (int i = 16; i < 64; ++i) {
        m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + ep1(e) + ch(e,f,g) + k256[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a=t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85; ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c; ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[32]) {
    size_t i = ctx->datalen;
    // Pad
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    // Append length
    for (int j = 0; j < 8; ++j) {
        ctx->data[63 - j] = uint8_t((ctx->bitlen >> (j * 8)) & 0xFF);
    }
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0xFF;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0xFF;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0xFF;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xFF;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xFF;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xFF;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xFF;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xFF;
    }
}

static void hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t out[32]) {
    const size_t block = 64;
    uint8_t kopad[64], kipad[64], k0[64];
    uint8_t tk[32];
    if (key_len > block) {
        SHA256_CTX c;
        sha256_init(&c);
        sha256_update(&c, key, key_len);
        sha256_final(&c, tk);
        memset(k0, 0, 64);
        memcpy(k0, tk, 32);
    } else {
        memset(k0, 0, 64);
        if (key_len > 0) memcpy(k0, key, key_len);
    }
    for (size_t i = 0; i < 64; ++i) {
        kipad[i] = k0[i] ^ 0x36;
        kopad[i] = k0[i] ^ 0x5c;
    }
    // inner
    SHA256_CTX ci;
    sha256_init(&ci);
    sha256_update(&ci, kipad, 64);
    if (data_len > 0) sha256_update(&ci, data, data_len);
    uint8_t inner[32];
    sha256_final(&ci, inner);
    // outer
    SHA256_CTX co;
    sha256_init(&co);
    sha256_update(&co, kopad, 64);
    sha256_update(&co, inner, 32);
    sha256_final(&co, out);
    // Wipe
    memset(tk, 0, sizeof(tk));
    memset(k0, 0, sizeof(k0));
    memset(kipad, 0, sizeof(kipad));
    memset(kopad, 0, sizeof(kopad));
    memset(inner, 0, sizeof(inner));
}

static void pbkdf2_hmac_sha256(const uint8_t *password, size_t password_len,
                               const uint8_t *salt, size_t salt_len,
                               uint32_t iterations, uint8_t *out, size_t dkLen) {
    uint32_t l = (uint32_t)((dkLen + 31) / 32);
    uint32_t r = (uint32_t)(dkLen - (l - 1) * 32);
    vector<uint8_t> U(32), T(32);
    vector<uint8_t> si(salt_len + 4);
    memcpy(si.data(), salt, salt_len);
    for (uint32_t i = 1; i <= l; ++i) {
        si[salt_len + 0] = (uint8_t)((i >> 24) & 0xFF);
        si[salt_len + 1] = (uint8_t)((i >> 16) & 0xFF);
        si[salt_len + 2] = (uint8_t)((i >> 8) & 0xFF);
        si[salt_len + 3] = (uint8_t)(i & 0xFF);
        hmac_sha256(password, password_len, si.data(), si.size(), U.data());
        memcpy(T.data(), U.data(), 32);
        for (uint32_t j = 1; j < iterations; ++j) {
            hmac_sha256(password, password_len, U.data(), 32, U.data());
            for (int k = 0; k < 32; ++k) T[k] ^= U[k];
        }
        size_t to_copy = (i == l) ? r : 32;
        memcpy(out + (i - 1) * 32, T.data(), to_copy);
    }
    // Wipe
    fill(U.begin(), U.end(), 0);
    fill(T.begin(), T.end(), 0);
    fill(si.begin(), si.end(), 0);
}

static string to_hex(const vector<uint8_t>& v) {
    static const char* hex = "0123456789abcdef";
    string s; s.resize(v.size() * 2);
    for (size_t i = 0; i < v.size(); ++i) {
        s[2*i] = hex[(v[i] >> 4) & 0xF];
        s[2*i+1] = hex[v[i] & 0xF];
    }
    return s;
}

static string to_hex_bytes(const uint8_t* v, size_t n) {
    static const char* hex = "0123456789abcdef";
    string s; s.resize(n * 2);
    for (size_t i = 0; i < n; ++i) {
        s[2*i] = hex[(v[i] >> 4) & 0xF];
        s[2*i+1] = hex[v[i] & 0xF];
    }
    return s;
}

static vector<uint8_t> from_hex(const string& hex) {
    if (hex.size() % 2 != 0) return {};
    vector<uint8_t> out(hex.size()/2);
    auto hexval = [](char c)->int{
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < out.size(); ++i) {
        int hi = hexval(hex[2*i]);
        int lo = hexval(hex[2*i+1]);
        if (hi < 0 || lo < 0) return {};
        out[i] = uint8_t((hi << 4) | lo);
    }
    return out;
}

static bool constant_time_eq(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i = 0; i < a.size(); ++i) r |= (a[i] ^ b[i]);
    return r == 0;
}

// Secure random bytes using std::random_device
static bool secure_random_bytes(uint8_t* out, size_t len) {
    std::random_device rd;
    for (size_t i = 0; i < len; ++i) {
        out[i] = static_cast<uint8_t>(rd());
    }
    return true;
}

// Password hashing parameters
static const uint32_t ITERATIONS = 210000;
static const size_t SALT_LEN = 16;
static const size_t KEY_LEN = 32;

// In-memory "database"
static unordered_map<string, string> DATABASE;

static bool is_valid_username(const string& u) {
    if (u.size() < 3 || u.size() > 64) return false;
    for (char c : u) {
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}
static bool is_valid_password(const string& p) {
    if (p.size() < 8) return false;
    bool lower=false, upper=false, digit=false, special=false;
    for (unsigned char c : p) {
        if (islower(c)) lower = true;
        else if (isupper(c)) upper = true;
        else if (isdigit(c)) digit = true;
        else special = true;
    }
    int cat = (lower?1:0)+(upper?1:0)+(digit?1:0)+(special?1:0);
    return cat >= 3;
}

static string hash_password(const string& password) {
    uint8_t salt[SALT_LEN];
    if (!secure_random_bytes(salt, SALT_LEN)) return {};
    vector<uint8_t> dk(KEY_LEN);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       salt, SALT_LEN, ITERATIONS, dk.data(), dk.size());
    string salt_hex = to_hex_bytes(salt, SALT_LEN);
    string hash_hex = to_hex(dk);
    memset(salt, 0, sizeof(salt));
    fill(dk.begin(), dk.end(), 0);
    return string("pbkdf2_sha256:") + to_string(ITERATIONS) + ":" + salt_hex + ":" + hash_hex;
}

static bool verify_password_hash(const string& stored, const string& password) {
    vector<string> parts;
    {
        string s = stored;
        size_t pos = 0;
        while (true) {
            size_t p = s.find(':', pos);
            if (p == string::npos) { parts.push_back(s.substr(pos)); break; }
            parts.push_back(s.substr(pos, p - pos));
            pos = p + 1;
            if (parts.size() == 3) { parts.push_back(s.substr(pos)); break; }
        }
    }
    if (parts.size() != 4) return false;
    if (!equal(parts[0].begin(), parts[0].end(), "pbkdf2_sha256")) return false;
    uint32_t iters = 0;
    try { iters = static_cast<uint32_t>(stoul(parts[1])); } catch (...) { return false; }
    vector<uint8_t> salt = from_hex(parts[2]);
    vector<uint8_t> expected = from_hex(parts[3]);
    if (salt.empty() || expected.empty()) return false;
    vector<uint8_t> dk(expected.size());
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       salt.data(), salt.size(), iters, dk.data(), dk.size());
    bool ok = constant_time_eq(expected, dk);
    fill(dk.begin(), dk.end(), 0);
    fill(salt.begin(), salt.end(), 0);
    fill(expected.begin(), expected.end(), 0);
    return ok;
}

bool update_user_password(const string& username, const string& new_password) {
    if (!is_valid_username(username) || !is_valid_password(new_password)) return false;
    string h = hash_password(new_password);
    if (h.empty()) return false;
    DATABASE[username] = std::move(h);
    return true;
}

string get_stored_hash(const string& username) {
    auto it = DATABASE.find(username);
    if (it == DATABASE.end()) return string();
    return it->second;
}

int main() {
    vector<pair<string,string>> tests = {
        {"alice", "CorrectHorseBatteryStaple!"},
        {"bob_123", "s3cureP@ssw0rd"},
        {"charlie", "anotherStrong#Pass1"},
        {"invalid user", "weak pass"},
        {"dave", "short7!"}
    };
    for (auto& t : tests) {
        cout << "Update " << t.first << " -> " << (update_user_password(t.first, t.second) ? "true" : "false") << "\n";
    }
    cout << "Stored alice: " << (!get_stored_hash("alice").empty() ? "present" : "none") << "\n";
    cout << "Verify alice correct: " << (verify_password_hash(get_stored_hash("alice"), "CorrectHorseBatteryStaple!") ? "true" : "false") << "\n";
    cout << "Verify alice wrong: " << (verify_password_hash(get_stored_hash("alice"), "WrongPassword!") ? "true" : "false") << "\n";
    cout << "Stored invalid user: " << (!get_stored_hash("invalid user").empty() ? "present" : "none") << "\n";
    return 0;
}