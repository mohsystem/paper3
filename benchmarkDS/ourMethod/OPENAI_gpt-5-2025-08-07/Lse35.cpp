#include <bits/stdc++.h>
using namespace std;

// Constants
static const size_t SALT_LEN = 16;
static const size_t HASH_LEN = 32; // 256-bit
static const uint32_t ITERATIONS = 210000;
static const size_t MAX_USERNAME_LEN = 50;
static const size_t MAX_PASSWORD_LEN = 128;

// Login HTML (simulation)
static const char* LOGIN_HTML =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head><meta charset=\"UTF-8\"><title>Login</title></head>\n"
"<body>\n"
"  <h1>Login</h1>\n"
"  <form action=\"/login\" method=\"POST\">\n"
"    <label>Username: <input type=\"text\" name=\"username\" maxlength=\"50\" required></label><br>\n"
"    <label>Password: <input type=\"password\" name=\"password\" maxlength=\"128\" required></label><br>\n"
"    <button type=\"submit\">Login</button>\n"
"  </form>\n"
"</body>\n"
"</html>\n";

// Secure zero
static void secure_zero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

// Random bytes using std::random_device
static bool random_bytes(uint8_t* out, size_t len) {
    std::random_device rd;
    for (size_t i = 0; i < len; ++i) {
        out[i] = static_cast<uint8_t>(rd());
    }
    return true;
}

// SHA-256 implementation
struct SHA256 {
    uint64_t bitlen;
    uint32_t state[8];
    uint8_t data[64];
    size_t datalen;

    SHA256() { init(); }

    static uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static uint32_t ep0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static uint32_t ep1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static uint32_t sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static uint32_t sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    void init() {
        datalen = 0;
        bitlen = 0;
        state[0] = 0x6a09e667;
        state[1] = 0xbb67ae85;
        state[2] = 0x3c6ef372;
        state[3] = 0xa54ff53a;
        state[4] = 0x510e527f;
        state[5] = 0x9b05688c;
        state[6] = 0x1f83d9ab;
        state[7] = 0x5be0cd19;
    }

    void transform(const uint8_t data[]) {
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
        uint32_t m[64];
        for (int i = 0; i < 16; ++i) {
            m[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | (data[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16];
        }
        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];
        uint32_t e = state[4];
        uint32_t f = state[5];
        uint32_t g = state[6];
        uint32_t h = state[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + ep1(e) + ch(e, f, g) + k[i] + m[i];
            uint32_t t2 = ep0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    void update(const uint8_t* in, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            data[datalen] = in[i];
            datalen++;
            if (datalen == 64) {
                transform(data);
                bitlen += 512;
                datalen = 0;
            }
        }
    }

    void final(uint8_t* out) {
        size_t i = datalen;

        // Pad
        if (datalen < 56) {
            data[i++] = 0x80;
            while (i < 56) data[i++] = 0x00;
        } else {
            data[i++] = 0x80;
            while (i < 64) data[i++] = 0x00;
            transform(data);
            memset(data, 0, 56);
        }

        bitlen += datalen * 8;
        data[63] = (uint8_t)(bitlen);
        data[62] = (uint8_t)(bitlen >> 8);
        data[61] = (uint8_t)(bitlen >> 16);
        data[60] = (uint8_t)(bitlen >> 24);
        data[59] = (uint8_t)(bitlen >> 32);
        data[58] = (uint8_t)(bitlen >> 40);
        data[57] = (uint8_t)(bitlen >> 48);
        data[56] = (uint8_t)(bitlen >> 56);
        transform(data);

        for (int j = 0; j < 8; ++j) {
            out[j * 4] = (uint8_t)((state[j] >> 24) & 0xff);
            out[j * 4 + 1] = (uint8_t)((state[j] >> 16) & 0xff);
            out[j * 4 + 2] = (uint8_t)((state[j] >> 8) & 0xff);
            out[j * 4 + 3] = (uint8_t)(state[j] & 0xff);
        }
    }
};

static void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
    SHA256 ctx;
    ctx.update(data, len);
    ctx.final(out);
}

// HMAC-SHA256
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]) {
    const size_t block = 64;
    uint8_t k0[64];
    memset(k0, 0, sizeof(k0));
    if (keylen > block) {
        uint8_t kh[32];
        sha256(key, keylen, kh);
        memcpy(k0, kh, 32);
        secure_zero(kh, sizeof(kh));
    } else {
        memcpy(k0, key, keylen);
    }
    uint8_t ipad[64], opad[64];
    for (size_t i = 0; i < 64; ++i) {
        ipad[i] = k0[i] ^ 0x36;
        opad[i] = k0[i] ^ 0x5c;
    }
    // inner
    vector<uint8_t> inner;
    inner.reserve(64 + datalen);
    inner.insert(inner.end(), ipad, ipad + 64);
    inner.insert(inner.end(), data, data + datalen);
    uint8_t ihash[32];
    sha256(inner.data(), inner.size(), ihash);
    // outer
    vector<uint8_t> outer;
    outer.reserve(64 + 32);
    outer.insert(outer.end(), opad, opad + 64);
    outer.insert(outer.end(), ihash, ihash + 32);
    sha256(outer.data(), outer.size(), out);

    secure_zero(k0, sizeof(k0));
    secure_zero(ipad, sizeof(ipad));
    secure_zero(opad, sizeof(opad));
    secure_zero(ihash, sizeof(ihash));
    secure_zero(inner.data(), inner.size());
    secure_zero(outer.data(), outer.size());
}

// PBKDF2-HMAC-SHA256
static void pbkdf2_hmac_sha256(const uint8_t* pwd, size_t pwdlen,
                               const uint8_t* salt, size_t saltlen,
                               uint32_t iters, uint8_t* out, size_t outlen) {
    size_t blocks = (outlen + 31) / 32;
    vector<uint8_t> asalt(salt, salt + saltlen);
    asalt.resize(saltlen + 4);

    for (size_t i = 1; i <= blocks; ++i) {
        asalt[saltlen + 0] = (uint8_t)((i >> 24) & 0xff);
        asalt[saltlen + 1] = (uint8_t)((i >> 16) & 0xff);
        asalt[saltlen + 2] = (uint8_t)((i >> 8) & 0xff);
        asalt[saltlen + 3] = (uint8_t)(i & 0xff);

        uint8_t u[32];
        hmac_sha256(pwd, pwdlen, asalt.data(), asalt.size(), u);

        uint8_t t[32];
        memcpy(t, u, 32);

        for (uint32_t j = 1; j < iters; ++j) {
            hmac_sha256(pwd, pwdlen, u, 32, u);
            for (size_t k = 0; k < 32; ++k) t[k] ^= u[k];
        }

        size_t offset = (i - 1) * 32;
        size_t cp = min((size_t)32, outlen - offset);
        memcpy(out + offset, t, cp);

        secure_zero(u, sizeof(u));
        secure_zero(t, sizeof(t));
    }
    secure_zero(asalt.data(), asalt.size());
}

struct UserRecord {
    vector<uint8_t> salt;
    vector<uint8_t> hash;
};

static unordered_map<string, UserRecord> DATABASE;

static bool valid_username(const string& u) {
    if (u.size() < 3 || u.size() > MAX_USERNAME_LEN) return false;
    for (char c : u) {
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}

static bool strong_password(const string& p) {
    if (p.size() < 12 || p.size() > MAX_PASSWORD_LEN) return false;
    bool up = false, lo = false, di = false, sp = false;
    for (unsigned char c : p) {
        if (iscntrl(c)) return false;
        if (isupper(c)) up = true;
        else if (islower(c)) lo = true;
        else if (isdigit(c)) di = true;
        else sp = true;
    }
    return up && lo && di && sp;
}

static void create_user(const string& username, const string& password) {
    if (!valid_username(username)) throw runtime_error("Invalid username");
    if (!strong_password(password)) throw runtime_error("Weak password");
    if (DATABASE.find(username) != DATABASE.end()) throw runtime_error("User exists");

    vector<uint8_t> salt(SALT_LEN);
    random_bytes(salt.data(), salt.size());

    vector<uint8_t> hash(HASH_LEN);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       salt.data(), salt.size(), ITERATIONS, hash.data(), hash.size());

    DATABASE[username] = UserRecord{salt, hash};
}

static bool ct_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    uint8_t r = 0;
    for (size_t i = 0; i < n; ++i) r |= (a[i] ^ b[i]);
    return r == 0;
}

static string login(const string& username, const string& password) {
    if (!valid_username(username)) return "login.html";
    if (password.empty() || password.size() > MAX_PASSWORD_LEN) return "login.html";

    auto it = DATABASE.find(username);
    if (it == DATABASE.end()) return "login.html";

    const auto& rec = it->second;
    vector<uint8_t> comp(HASH_LEN);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       rec.salt.data(), rec.salt.size(), ITERATIONS, comp.data(), comp.size());
    bool ok = ct_equal(comp.data(), rec.hash.data(), HASH_LEN);
    secure_zero(comp.data(), comp.size());
    return ok ? "home.html" : "login.html";
}

static const string& get_login_html() {
    static const string s(LOGIN_HTML);
    return s;
}

int main() {
    cout << get_login_html() << "\n";

    try {
        create_user("user1", "StrongPassw0rd!");
        create_user("user2", "Compl3x!Pass#2");
        create_user("user3", "An0ther$trongOne");
    } catch (const exception& e) {
        cerr << "Setup error\n";
        return 1;
    }

    // 5 test cases
    vector<pair<string, string>> tests = {
        {"user1", "StrongPassw0rd!"},
        {"user2", "wrongPassword"},
        {"nosuchuser", "Anything1!"},
        {"bad user", "Compl3x!Pass#2"},
        {"user3", string(129, 'x')}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << ": " << login(tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}