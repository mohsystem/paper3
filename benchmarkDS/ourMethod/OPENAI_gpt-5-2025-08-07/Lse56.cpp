#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256, HMAC-SHA256, PBKDF2-HMAC-SHA256 implementation (public domain-style)
// This is a straightforward implementation suitable for demo purposes.

struct SHA256Ctx {
    uint32_t state[8];
    uint64_t bitcount;
    uint8_t buffer[64];
};

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

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

static void sha256_init(SHA256Ctx* ctx) {
    ctx->state[0]=0x6a09e667ul; ctx->state[1]=0xbb67ae85ul; ctx->state[2]=0x3c6ef372ul; ctx->state[3]=0xa54ff53aul;
    ctx->state[4]=0x510e527ful; ctx->state[5]=0x9b05688cul; ctx->state[6]=0x1f83d9abul; ctx->state[7]=0x5be0cd19ul;
    ctx->bitcount = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

static void sha256_transform(SHA256Ctx* ctx, const uint8_t data[64]) {
    uint32_t w[64];
    for (int i=0;i<16;i++) {
        w[i] = (uint32_t(data[i*4])<<24) | (uint32_t(data[i*4+1])<<16) | (uint32_t(data[i*4+2])<<8) | uint32_t(data[i*4+3]);
    }
    for (int i=16;i<64;i++) {
        uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15]>>3);
        uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2]>>10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint32_t a=ctx->state[0],b=ctx->state[1],c=ctx->state[2],d=ctx->state[3];
    uint32_t e=ctx->state[4],f=ctx->state[5],g=ctx->state[6],h=ctx->state[7];
    for (int i=0;i<64;i++) {
        uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t temp1 = h + S1 + ch + K256[i] + w[i];
        uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;
        h=g; g=f; f=e; e=d + temp1; d=c; c=b; b=a; a=temp1 + temp2;
    }
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

static void sha256_update(SHA256Ctx* ctx, const uint8_t* data, size_t len) {
    size_t i = 0;
    size_t have = (ctx->bitcount >> 3) & 63;
    ctx->bitcount += uint64_t(len) * 8;
    size_t need = 64 - have;
    if (len >= need) {
        if (have) {
            memcpy(ctx->buffer + have, data, need);
            sha256_transform(ctx, ctx->buffer);
            i += need;
            have = 0;
        }
        for (; i + 63 < len; i += 64) {
            sha256_transform(ctx, data + i);
        }
    }
    if (i < len) {
        memcpy(ctx->buffer + have, data + i, len - i);
    }
}

static void sha256_final(SHA256Ctx* ctx, uint8_t out[32]) {
    uint8_t pad[64]; memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;
    uint8_t lenbuf[8];
    uint64_t bits = ctx->bitcount;
    for (int i=0;i<8;i++) lenbuf[7-i] = uint8_t(bits >> (i*8));
    size_t have = (ctx->bitcount >> 3) & 63;
    size_t padlen = (have < 56) ? (56 - have) : (120 - have);
    sha256_update(ctx, pad, padlen);
    sha256_update(ctx, lenbuf, 8);
    for (int i=0;i<8;i++) {
        out[i*4] = uint8_t(ctx->state[i] >> 24);
        out[i*4+1] = uint8_t(ctx->state[i] >> 16);
        out[i*4+2] = uint8_t(ctx->state[i] >> 8);
        out[i*4+3] = uint8_t(ctx->state[i]);
    }
    // Wipe context
    memset(ctx, 0, sizeof(*ctx));
}

static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t len, uint8_t out[32]) {
    uint8_t k_ipad[64], k_opad[64];
    uint8_t keyhash[32];
    if (keylen > 64) {
        SHA256Ctx c; sha256_init(&c); sha256_update(&c, key, keylen); sha256_final(&c, keyhash);
        key = keyhash; keylen = 32;
    }
    memset(k_ipad, 0x36, 64);
    memset(k_opad, 0x5c, 64);
    for (size_t i=0;i<keylen;i++) {
        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }
    uint8_t ihash[32];
    SHA256Ctx ci; sha256_init(&ci); sha256_update(&ci, k_ipad, 64); sha256_update(&ci, data, len); sha256_final(&ci, ihash);
    SHA256Ctx co; sha256_init(&co); sha256_update(&co, k_opad, 64); sha256_update(&co, ihash, 32); sha256_final(&co, out);
    // Wipe
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memset(keyhash, 0, sizeof(keyhash));
    memset(ihash, 0, sizeof(ihash));
}

static void pbkdf2_hmac_sha256(const uint8_t* password, size_t passlen, const uint8_t* salt, size_t saltlen, uint32_t iter, uint8_t* out, size_t dklen) {
    // General PBKDF2 for any dklen
    uint32_t block_count = (uint32_t)((dklen + 31) / 32);
    vector<uint8_t> asalt(salt, salt + saltlen);
    asalt.resize(saltlen + 4);
    size_t pos = 0;
    for (uint32_t i = 1; i <= block_count; i++) {
        asalt[saltlen+0] = (uint8_t)(i >> 24);
        asalt[saltlen+1] = (uint8_t)(i >> 16);
        asalt[saltlen+2] = (uint8_t)(i >> 8);
        asalt[saltlen+3] = (uint8_t)(i);
        uint8_t U[32];
        hmac_sha256(password, passlen, asalt.data(), asalt.size(), U);
        uint8_t T[32];
        memcpy(T, U, 32);
        for (uint32_t j = 1; j < iter; j++) {
            hmac_sha256(password, passlen, U, 32, U);
            for (int k=0;k<32;k++) T[k] ^= U[k];
        }
        size_t l = min((size_t)32, dklen - pos);
        memcpy(out + pos, T, l);
        pos += l;
        memset(U, 0, sizeof(U));
        memset(T, 0, sizeof(T));
    }
    // wipe asalt
    fill(asalt.begin(), asalt.end(), 0);
}

static bool constant_time_eq(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t r = 0;
    for (size_t i=0;i<a.size();i++) r |= (a[i] ^ b[i]);
    return r == 0;
}

struct User {
    string username;
    string email;
    vector<uint8_t> salt;
    vector<uint8_t> hash;
    uint32_t iterations;
    bool loggedIn;
};

static unordered_map<string, User> USERS;
static const size_t SALT_LEN = 16;
static const size_t DK_LEN = 32;
static const uint32_t PBKDF2_ITER = 210000;

static bool validateUsername(const string& username) {
    if (username.size() < 3 || username.size() > 50) return false;
    for (char c : username) {
        if (!(isalnum((unsigned char)c) || c=='_' || c=='-' || c=='.')) return false;
    }
    return true;
}

static bool validateEmail(const string& email) {
    if (email.size() < 6 || email.size() > 254) return false;
    if (email.find(' ') != string::npos) return false;
    size_t at = email.find('@');
    if (at == string::npos || at == 0 || at != email.rfind('@')) return false;
    string local = email.substr(0, at);
    string domain = email.substr(at+1);
    if (local.empty() || domain.size() < 3) return false;
    if (domain.find('.') == string::npos) return false;
    return true;
}

static bool isStrongPassword(const string& pw) {
    if (pw.size() < 12 || pw.size() > 128) return false;
    bool hasU=false, hasL=false, hasD=false, hasS=false;
    for (char c : pw) {
        if (isspace((unsigned char)c)) return false;
        if (isupper((unsigned char)c)) hasU = true;
        else if (islower((unsigned char)c)) hasL = true;
        else if (isdigit((unsigned char)c)) hasD = true;
        else if (string("!@#$%^&*()-_=+[]{};:,.?/\\|`~'\"<>").find(c) != string::npos) hasS = true;
    }
    return hasU && hasL && hasD && hasS;
}

static void randomBytes(uint8_t* dst, size_t len) {
    // Use std::random_device to fill; not ideal on all platforms but acceptable for demo
    std::random_device rd;
    for (size_t i=0;i<len;i++) dst[i] = static_cast<uint8_t>(rd());
}

static vector<uint8_t> deriveKey(const string& password, const vector<uint8_t>& salt, uint32_t iter, size_t dkLen) {
    vector<uint8_t> out(dkLen);
    pbkdf2_hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(),
                       salt.data(), salt.size(), iter, out.data(), out.size());
    return out;
}

static bool registerUser(const string& username, const string& email, const string& password) {
    if (!validateUsername(username) || !validateEmail(email)) return false;
    if (!isStrongPassword(password)) return false;
    if (USERS.find(username) != USERS.end()) return false;
    vector<uint8_t> salt(SALT_LEN);
    randomBytes(salt.data(), salt.size());
    vector<uint8_t> hash = deriveKey(password, salt, PBKDF2_ITER, DK_LEN);
    User u{username, email, salt, hash, PBKDF2_ITER, false};
    USERS.emplace(username, std::move(u));
    return true;
}

static bool verifyPassword(const User& u, const string& provided) {
    vector<uint8_t> cand = deriveKey(provided, u.salt, u.iterations, DK_LEN);
    return constant_time_eq(u.hash, cand);
}

static bool loginUser(const string& username, const string& password) {
    auto it = USERS.find(username);
    if (it == USERS.end()) return false;
    bool ok = verifyPassword(it->second, password);
    if (ok) it->second.loggedIn = true;
    return ok;
}

static bool changeEmail(const string& username, const string& oldEmail, const string& newEmail, const string& confirmPassword) {
    auto it = USERS.find(username);
    if (it == USERS.end()) return false;
    User& u = it->second;
    if (!u.loggedIn) return false;
    if (!validateEmail(oldEmail) || !validateEmail(newEmail)) return false;
    if (u.email != oldEmail) return false;
    if (u.email == newEmail) return false;
    if (!verifyPassword(u, confirmPassword)) return false;
    u.email = newEmail;
    return true;
}

static string generateStrongRandomPassword(size_t length = 16) {
    if (length < 12) length = 12;
    string lowers = "abcdefghijklmnopqrstuvwxyz";
    string uppers = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string digits = "0123456789";
    string symbols = "!@#$%^&*()-_=+[]{};:,.?/\\|`~'\"<>";
    string all = lowers + uppers + digits + symbols;
    std::random_device rd;
    auto pick = [&](const string& s)->char { return s[rd() % s.size()]; };
    string pw;
    pw.push_back(pick(lowers));
    pw.push_back(pick(uppers));
    pw.push_back(pick(digits));
    pw.push_back(pick(symbols));
    while (pw.size() < length) pw.push_back(pick(all));
    // Fisher-Yates shuffle
    for (size_t i = pw.size()-1; i > 0; --i) {
        size_t j = rd() % (i+1);
        swap(pw[i], pw[j]);
    }
    return pw;
}

int main() {
    string username = "alice";
    string initialEmail = "alice@example.com";
    string strongPassword = generateStrongRandomPassword(16);

    cout << "Register user: " << (registerUser(username, initialEmail, strongPassword) ? "true" : "false") << "\n";

    // Test 1: change without login
    cout << "Test1 change without login (expect false): "
         << (changeEmail(username, initialEmail, "alice_new@example.com", strongPassword) ? "true" : "false") << "\n";

    // Test 2: login wrong password
    cout << "Test2 login wrong password (expect false): "
         << (loginUser(username, string("WrongPassw0rd!")) ? "true" : "false") << "\n";

    // Test 3: login correct password
    cout << "Test3 login correct password (expect true): "
         << (loginUser(username, strongPassword) ? "true" : "false") << "\n";

    // Test 4: change to same email
    cout << "Test4 change to same email (expect false): "
         << (changeEmail(username, initialEmail, initialEmail, strongPassword) ? "true" : "false") << "\n";

    // Test 5: successful email change
    cout << "Test5 change email valid (expect true): "
         << (changeEmail(username, initialEmail, "alice_secure@example.net", strongPassword) ? "true" : "false") << "\n";

    return 0;
}