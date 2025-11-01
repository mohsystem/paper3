#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <cstdint>
#include <cstring>

struct Credential {
    std::vector<uint8_t> salt;
    std::vector<uint8_t> hash;
    uint32_t iterations;
};

static std::string index_page() {
    return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body><h1>Welcome</h1><a href=\"/login\">Login</a></body></html>";
}

static std::string login_page() {
    return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body><h1>Login</h1><form action=\"/do_login\" method=\"post\">Username: <input type=\"text\" name=\"username\" maxlength=\"32\" /><br/>Password: <input type=\"password\" name=\"password\" maxlength=\"128\" /><br/><button type=\"submit\">Login</button></form></body></html>";
}

static std::string secret_page(const std::unordered_map<std::string, std::string>& session) {
    auto it = session.find("username");
    if (it != session.end() && !it->second.empty()) {
        return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>Secret settings</body></html>";
    }
    return "<!doctype html><html><head><meta charset=\"utf-8\"></head><body>Unauthorized</body></html>";
}

static bool is_valid_username(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) return false;
    for (char c : u) {
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static bool is_valid_password(const std::string& p) {
    return p.size() >= 12 && p.size() <= 128;
}

/* ---- Minimal SHA-256 implementation ---- */
typedef struct {
    uint8_t data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

#define ROTLEFT(a,b)  (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z)     (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z)    (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)        (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)        (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)       (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)       (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

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
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i=0,j=0; i<16; ++i, j+=4)
        m[i] = ( (uint32_t)data[j] << 24 ) | ( (uint32_t)data[j+1] << 16 ) | ( (uint32_t)data[j+2] << 8 ) | ( (uint32_t)data[j+3] );
    for ( ; i<64; ++i)
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];

    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    for (i=0; i<64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + k256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d + t1;
        d=c; c=b; b=a; a=t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0]=0x6a09e667;
    ctx->state[1]=0xbb67ae85;
    ctx->state[2]=0x3c6ef372;
    ctx->state[3]=0xa54ff53a;
    ctx->state[4]=0x510e527f;
    ctx->state[5]=0x9b05688c;
    ctx->state[6]=0x1f83d9ab;
    ctx->state[7]=0x5be0cd19;
}

static void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i=0; i<len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

static void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    size_t i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        std::memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8ULL;
    ctx->data[63] = (uint8_t)(ctx->bitlen      );
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8 );
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i=0; i<4; ++i) {
        for (int j=0; j<8; ++j) {
            hash[i + (j*4)] = (uint8_t)((ctx->state[j] >> (24 - i*8)) & 0xFF);
        }
    }
}

/* HMAC-SHA256 */
static void hmac_sha256(const uint8_t* key, size_t keylen, const uint8_t* data, size_t datalen, uint8_t out[32]) {
    uint8_t k_ipad[64]; std::memset(k_ipad, 0x36, sizeof(k_ipad));
    uint8_t k_opad[64]; std::memset(k_opad, 0x5c, sizeof(k_opad));
    uint8_t tk[32];

    if (keylen > 64) {
        SHA256_CTX tctx;
        sha256_init(&tctx);
        sha256_update(&tctx, key, keylen);
        sha256_final(&tctx, tk);
        key = tk;
        keylen = 32;
    }

    uint8_t kopad[64]; std::memset(kopad, 0, sizeof(kopad));
    uint8_t kipad[64]; std::memset(kipad, 0, sizeof(kipad));
    std::memcpy(kopad, key, keylen);
    std::memcpy(kipad, key, keylen);

    for (size_t i=0;i<64;i++){ kopad[i] ^= 0x5c; kipad[i] ^= 0x36; }

    SHA256_CTX ctx;
    uint8_t inner_hash[32];

    sha256_init(&ctx);
    sha256_update(&ctx, kipad, 64);
    sha256_update(&ctx, data, datalen);
    sha256_final(&ctx, inner_hash);

    sha256_init(&ctx);
    sha256_update(&ctx, kopad, 64);
    sha256_update(&ctx, inner_hash, 32);
    sha256_final(&ctx, out);

    std::memset(tk, 0, sizeof(tk));
    std::memset(kopad, 0, sizeof(kopad));
    std::memset(kipad, 0, sizeof(kipad));
}

/* PBKDF2-HMAC-SHA256 */
static void pbkdf2_hmac_sha256(const std::string& password, const std::vector<uint8_t>& salt, uint32_t iterations, uint8_t* out, size_t dkLen) {
    const size_t hLen = 32;
    size_t l = (dkLen + hLen - 1) / hLen;
    size_t r = dkLen - (l - 1) * hLen;

    std::vector<uint8_t> asalt(salt);
    asalt.resize(salt.size() + 4);

    std::vector<uint8_t> U(hLen);
    std::vector<uint8_t> T(hLen);

    for (size_t i = 1; i <= l; i++) {
        asalt[salt.size()]   = (uint8_t)((i >> 24) & 0xff);
        asalt[salt.size()+1] = (uint8_t)((i >> 16) & 0xff);
        asalt[salt.size()+2] = (uint8_t)((i >> 8) & 0xff);
        asalt[salt.size()+3] = (uint8_t)(i & 0xff);

        hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(), asalt.data(), asalt.size(), U.data());
        std::memcpy(T.data(), U.data(), hLen);

        for (uint32_t j = 1; j < iterations; j++) {
            hmac_sha256(reinterpret_cast<const uint8_t*>(password.data()), password.size(), U.data(), hLen, U.data());
            for (size_t k = 0; k < hLen; k++) {
                T[k] ^= U[k];
            }
        }

        size_t clen = (i == l) ? r : hLen;
        std::memcpy(out + (i - 1) * hLen, T.data(), clen);
    }
}

/* Constant-time equals */
static bool constant_time_equals(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); i++) diff |= (uint8_t)(a[i] ^ b[i]);
    return diff == 0;
}

static std::vector<uint8_t> gen_salt(size_t n) {
    std::vector<uint8_t> s(n);
    std::random_device rd;
    for (size_t i = 0; i < n; i++) {
        s[i] = (uint8_t)(rd() & 0xFF);
    }
    return s;
}

static Credential create_credential(const std::string& password) {
    Credential c;
    c.iterations = 210000;
    c.salt = gen_salt(16);
    c.hash.resize(32);
    pbkdf2_hmac_sha256(password, c.salt, c.iterations, c.hash.data(), c.hash.size());
    return c;
}

static bool verify_password(const std::string& password, const Credential& c) {
    std::vector<uint8_t> out(c.hash.size());
    pbkdf2_hmac_sha256(password, c.salt, c.iterations, out.data(), out.size());
    return constant_time_equals(out, c.hash);
}

static std::string do_login(std::unordered_map<std::string, std::string>& session,
                            const std::string& username,
                            const std::string& password,
                            const std::unordered_map<std::string, Credential>& users) {
    if (!is_valid_username(username)) return "ERROR:Invalid username";
    if (!is_valid_password(password)) return "ERROR:Invalid password";
    auto it = users.find(username);
    if (it == users.end()) return "ERROR:Invalid credentials";
    if (!verify_password(password, it->second)) return "ERROR:Invalid credentials";
    session["username"] = username;
    return "REDIRECT:/secret";
}

/* Random password generator for tests */
static std::string random_password(size_t n) {
    const std::string alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*()-_=+";
    std::random_device rd;
    std::string out;
    out.reserve(n);
    for (size_t i = 0; i < n; i++) {
        out.push_back(alphabet[rd() % alphabet.size()]);
    }
    return out;
}

int main() {
    std::unordered_map<std::string, Credential> users;
    std::string username = "alice";
    std::string password = random_password(14);
    users[username] = create_credential(password);

    std::unordered_map<std::string, std::string> session;

    int passed = 0;
    int total = 5;

    // Test 1: Success
    std::string r1 = do_login(session, username, password, users);
    bool t1 = (r1 == "REDIRECT:/secret") &&
              (session.find("username") != session.end() && session["username"] == username) &&
              (secret_page(session).find("Secret settings") != std::string::npos);
    if (t1) passed++;
    session.clear();

    // Test 2: Wrong password
    std::string r2 = do_login(session, username, "WrongPassword123!", users);
    bool t2 = (r2.rfind("ERROR:", 0) == 0) && (session.find("username") == session.end());
    if (t2) passed++;

    // Test 3: Wrong username
    std::string r3 = do_login(session, "bob", password, users);
    bool t3 = (r3.rfind("ERROR:", 0) == 0) && (session.find("username") == session.end());
    if (t3) passed++;

    // Test 4: Invalid username format
    std::string r4 = do_login(session, "bad<user>", password, users);
    bool t4 = (r4 == "ERROR:Invalid username");
    if (t4) passed++;

    // Test 5: Short password
    std::string r5 = do_login(session, username, "shortPwd1!", users);
    bool t5 = (r5 == "ERROR:Invalid password");
    if (t5) passed++;

    std::cout << "Index page sample: " << (index_page().find("Login") != std::string::npos ? "OK" : "FAIL") << "\n";
    std::cout << "Login page sample: " << (login_page().find("form") != std::string::npos ? "OK" : "FAIL") << "\n";
    std::cout << "Tests passed: " << passed << "/" << total << "\n";
    return 0;
}