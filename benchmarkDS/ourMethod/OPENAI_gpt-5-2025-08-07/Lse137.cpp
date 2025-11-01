#include <bits/stdc++.h>
using namespace std;

// Minimal secure implementation: SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256
// Randomness via /dev/urandom

// ===== Utility: secure zero =====
static void secure_zero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) *p++ = 0;
}

// ===== Random bytes =====
static bool get_random_bytes(uint8_t* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t r = fread(out, 1, len, f);
    fclose(f);
    return r == len;
}

// ===== SHA-256 implementation =====
struct SHA256Ctx {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t bsig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
static inline uint32_t bsig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
static inline uint32_t ssig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
static inline uint32_t ssig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

static const uint32_t K256[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_init(SHA256Ctx* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667; ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372; ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f; ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab; ctx->state[7]=0x5be0cd19;
}

static void sha256_transform(SHA256Ctx* ctx, const uint8_t data[]) {
    uint32_t m[64];
    for (int i=0;i<16;i++) {
        m[i] = (uint32_t)data[i*4]<<24 | (uint32_t)data[i*4+1]<<16 | (uint32_t)data[i*4+2]<<8 | (uint32_t)data[i*4+3];
    }
    for (int i=16;i<64;i++) {
        m[i] = ssig1(m[i-2]) + m[i-7] + ssig0(m[i-15]) + m[i-16];
    }
    uint32_t a=ctx->state[0], b=ctx->state[1], c=ctx->state[2], d=ctx->state[3];
    uint32_t e=ctx->state[4], f=ctx->state[5], g=ctx->state[6], h=ctx->state[7];
    for (int i=0;i<64;i++) {
        uint32_t t1 = h + bsig1(e) + ch(e,f,g) + K256[i] + m[i];
        uint32_t t2 = bsig0(a) + maj(a,b,c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_update(SHA256Ctx* ctx, const uint8_t* data, size_t len) {
    for (size_t i=0;i<len;i++) {
        ctx->data[ctx->datalen++] = data[i];
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256Ctx* ctx, uint8_t out[32]) {
    size_t i = ctx->datalen;
    ctx->data[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->data[i++] = 0;
        sha256_transform(ctx, ctx->data);
        i = 0;
    }
    while (i < 56) ctx->data[i++] = 0;
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (int j=0;j<8;j++) {
        out[j*4+0] = (uint8_t)(ctx->state[j] >> 24);
        out[j*4+1] = (uint8_t)(ctx->state[j] >> 16);
        out[j*4+2] = (uint8_t)(ctx->state[j] >> 8);
        out[j*4+3] = (uint8_t)(ctx->state[j]);
    }
}

static vector<uint8_t> sha256(const vector<uint8_t>& data) {
    SHA256Ctx ctx; sha256_init(&ctx);
    if (!data.empty()) sha256_update(&ctx, data.data(), data.size());
    vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

// ===== HMAC-SHA256 =====
static vector<uint8_t> hmac_sha256(const vector<uint8_t>& key, const vector<uint8_t>& msg) {
    const size_t block = 64;
    vector<uint8_t> k(block, 0);
    if (key.size() > block) {
        auto kh = sha256(key);
        memcpy(k.data(), kh.data(), kh.size());
    } else {
        memcpy(k.data(), key.data(), key.size());
    }
    vector<uint8_t> ipad(block, 0x36), opad(block, 0x5c);
    for (size_t i=0;i<block;i++) {
        ipad[i] ^= k[i];
        opad[i] ^= k[i];
    }
    vector<uint8_t> inner; inner.reserve(block + msg.size());
    inner.insert(inner.end(), ipad.begin(), ipad.end());
    inner.insert(inner.end(), msg.begin(), msg.end());
    auto ih = sha256(inner);
    vector<uint8_t> outer; outer.reserve(block + ih.size());
    outer.insert(outer.end(), opad.begin(), opad.end());
    outer.insert(outer.end(), ih.begin(), ih.end());
    return sha256(outer);
}

// ===== PBKDF2-HMAC-SHA256 =====
static void pbkdf2_hmac_sha256(const string& password, const vector<uint8_t>& salt, uint32_t iterations, size_t dkLen, vector<uint8_t>& out) {
    out.assign(dkLen, 0);
    vector<uint8_t> pw(password.begin(), password.end());
    uint32_t blocks = (uint32_t)((dkLen + 31) / 32);
    vector<uint8_t> U(32), T(32);
    vector<uint8_t> saltblk(salt);
    saltblk.resize(salt.size() + 4);
    for (uint32_t i=1; i<=blocks; i++) {
        saltblk[salt.size()+0] = (uint8_t)(i >> 24);
        saltblk[salt.size()+1] = (uint8_t)(i >> 16);
        saltblk[salt.size()+2] = (uint8_t)(i >> 8);
        saltblk[salt.size()+3] = (uint8_t)(i);
        U = hmac_sha256(pw, saltblk);
        T = U;
        for (uint32_t j=1; j<iterations; j++) {
            U = hmac_sha256(pw, U);
            for (size_t k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t offset = (size_t)(i-1)*32;
        size_t cp = min((size_t)32, dkLen - offset);
        memcpy(out.data() + offset, T.data(), cp);
    }
    secure_zero(U.data(), U.size());
    secure_zero(T.data(), T.size());
    secure_zero((void*)pw.data(), pw.size());
}

// ===== Utilities =====
static bool constant_time_eq(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i=0;i<a.size();i++) r |= (a[i] ^ b[i]);
    return r == 0;
}
static bool constant_time_eq_str(const string& a, const string& b) {
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i=0;i<a.size();i++) r |= (uint8_t)(a[i] ^ b[i]);
    return r == 0;
}
static bool validate_identifier(const string& s, size_t maxLen) {
    if (s.empty() || s.size() > maxLen) return false;
    for (char c: s) {
        if (!(isalnum((unsigned char)c) || c=='_' || c=='-' || c=='.' || c=='@')) return false;
    }
    return true;
}
static bool strong_password(const string& pw) {
    if (pw.size() < 12 || pw.size() > 256) return false;
    bool up=false, lo=false, di=false, sp=false;
    for (char c: pw) {
        if (isupper((unsigned char)c)) up=true;
        else if (islower((unsigned char)c)) lo=true;
        else if (isdigit((unsigned char)c)) di=true;
        else sp=true;
    }
    return up && lo && di && sp;
}
static string url_decode(const string& s) {
    string out; out.reserve(s.size());
    for (size_t i=0;i<s.size();i++) {
        if (s[i]=='%' && i+2<s.size() && isxdigit(s[i+1]) && isxdigit(s[i+2])) {
            string hex = s.substr(i+1,2);
            char ch = (char)strtol(hex.c_str(), nullptr, 16);
            out.push_back(ch);
            i+=2;
        } else if (s[i]=='+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}
static unordered_map<string,string> parse_params(const string& url) {
    unordered_map<string,string> m;
    auto q = url.find('?');
    if (q == string::npos) return m;
    string query = url.substr(q+1);
    size_t pos = 0;
    while (pos < query.size()) {
        size_t amp = query.find('&', pos);
        string pair = query.substr(pos, (amp==string::npos)?string::npos:(amp-pos));
        size_t eq = pair.find('=');
        if (eq != string::npos && eq > 0) {
            string k = url_decode(pair.substr(0, eq));
            string v = url_decode(pair.substr(eq+1));
            m[k] = v;
        }
        if (amp == string::npos) break;
        pos = amp + 1;
    }
    return m;
}
static string url_encode_safe(const string& s) {
    static const string safe = "-_.~";
    ostringstream oss;
    for (unsigned char c: s) {
        if (isalnum(c) || safe.find((char)c) != string::npos) {
            oss << (char)c;
        } else {
            oss << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)c
                << std::nouppercase << std::dec;
        }
    }
    return oss.str();
}
static string rand_token(size_t n) {
    vector<uint8_t> b(n);
    if (!get_random_bytes(b.data(), b.size())) b.assign(n, 0);
    ostringstream oss;
    for (auto v: b) oss << std::hex << std::setw(2) << std::setfill('0') << (int)v;
    return oss.str();
}

// ===== User and DB =====
struct User {
    string username;
    vector<uint8_t> salt;
    vector<uint8_t> hash;
    bool moderator;
    string session;
};
static unordered_map<string, User> DB;

static void add_user(const string& username, const char* pw_env, bool moderator, const char* sess_env) {
    if (!validate_identifier(username, 64)) return;
    if (!pw_env) return;
    string pw = pw_env;
    if (!strong_password(pw)) return;

    vector<uint8_t> salt(16);
    if (!get_random_bytes(salt.data(), salt.size())) return;

    vector<uint8_t> dk;
    pbkdf2_hmac_sha256(pw, salt, 210000, 32, dk);

    string session;
    if (sess_env && validate_identifier(sess_env, 128)) session = string(sess_env);
    else session = rand_token(16);

    User u{username, salt, dk, moderator, session};
    DB[username] = u;
    // Do not log secrets
}

static bool auth_mod(const string& username, const string& password, const string& session) {
    if (!validate_identifier(username, 64)) return false;
    if (password.empty() || password.size() > 256) return false;
    if (!validate_identifier(session, 128)) return false;

    auto it = DB.find(username);
    if (it == DB.end()) return false;
    const User& u = it->second;

    if (!constant_time_eq_str(u.session, session)) return false;

    vector<uint8_t> dk;
    pbkdf2_hmac_sha256(password, u.salt, 210000, 32, dk);
    bool ok = constant_time_eq(dk, u.hash);
    secure_zero(dk.data(), dk.size());
    return ok && u.moderator;
}

static bool check_mod(const string& url) {
    if (url.rfind("/check_mod", 0) != 0) return false;
    auto params = parse_params(url);
    auto itU = params.find("username");
    auto itP = params.find("password");
    auto itS = params.find("session");
    if (itU == params.end() || itP == params.end() || itS == params.end()) return false;
    return auth_mod(itU->second, itP->second, itS->second);
}

int main() {
    const char* mod_user = getenv("MOD_USER"); if (!mod_user) mod_user = "mod1";
    const char* mod_pass = getenv("MOD_PASS");
    const char* mod_sess = getenv("MOD_SESSION");

    const char* reg_user = getenv("USER_USER"); if (!reg_user) reg_user = "user1";
    const char* reg_pass = getenv("USER_PASS");
    const char* reg_sess = getenv("USER_SESSION");

    add_user(mod_user, mod_pass, true, mod_sess);
    add_user(reg_user, reg_pass, false, reg_sess);

    vector<string> tests;
    if (DB.find(mod_user) != DB.end() && mod_pass) {
        string sess = DB[mod_user].session;
        tests.push_back(string("/check_mod?username=") + mod_user + "&password=" + url_encode_safe(mod_pass) + "&session=" + sess);
    } else {
        tests.push_back(string("/check_mod?username=") + mod_user + "&password=WrongPass123!&session=invalid");
    }

    if (DB.find(mod_user) != DB.end()) {
        string sess = DB[mod_user].session;
        tests.push_back(string("/check_mod?username=") + mod_user + "&password=WrongPass123!&session=" + sess);
    } else {
        tests.push_back(string("/check_mod?username=") + mod_user + "&password=WrongPass123!&session=wrong");
    }

    if (DB.find(mod_user) != DB.end() && mod_pass) {
        tests.push_back(string("/check_mod?username=") + mod_user + "&password=" + url_encode_safe(mod_pass) + "&session=bad" + rand_token(4));
    } else {
        tests.push_back("/check_mod?username=nouser&password=SomethingStrong1!&session=bad");
    }

    tests.push_back("/check_mod?username=ghost&password=NopePass1!&session=anything123");

    if (DB.find(reg_user) != DB.end() && reg_pass) {
        string sess = DB[reg_user].session;
        tests.push_back(string("/check_mod?username=") + reg_user + "&password=" + url_encode_safe(reg_pass) + "&session=" + sess);
    } else {
        tests.push_back(string("/check_mod?username=") + reg_user + "&password=WrongPass123!&session=wrong");
    }

    for (size_t i=0;i<tests.size();i++) {
        bool res = check_mod(tests[i]);
        cout << "Test " << (i+1) << ": " << (res ? "true" : "false") << "\n";
    }
    return 0;
}