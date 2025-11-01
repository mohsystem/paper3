// Chain-of-Through Steps via comments.
// Note: Includes a minimal SHA-256 implementation to avoid external deps.

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <random>
#include <cstring>
#include <regex>

// --- Minimal SHA-256 implementation (public domain style) ---

struct SHA256_CTX {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
};

static inline uint32_t ROTRIGHT(uint32_t a, uint32_t b){ return ((a >> b) | (a << (32-b))); }
static inline uint32_t CH(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (~x & z); }
static inline uint32_t MAJ(uint32_t x, uint32_t y, uint32_t z){ return (x & y) ^ (x & z) ^ (y & z); }
static inline uint32_t EP0(uint32_t x){ return ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22); }
static inline uint32_t EP1(uint32_t x){ return ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25); }
static inline uint32_t SIG0(uint32_t x){ return ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3); }
static inline uint32_t SIG1(uint32_t x){ return ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10); }

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

static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t a,b,c,d,e,f,g,h,i,j,t1,t2,m[64];
    for (i=0, j=0; i<16; ++i, j+=4) {
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j+1] << 16) | ((uint32_t)data[j+2] << 8) | ((uint32_t)data[j+3]);
    }
    for (; i<64; ++i) {
        m[i] = SIG1(m[i-2]) + m[i-7] + SIG0(m[i-15]) + m[i-16];
    }
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i=0; i<64; ++i) {
        t1 = h + EP1(e) + CH(e,f,g) + K256[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g; g = f; f = e;
        e = d + t1;
        d = c; c = b; b = a;
        a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667ul;
    ctx->state[1] = 0xbb67ae85ul;
    ctx->state[2] = 0x3c6ef372ul;
    ctx->state[3] = 0xa54ff53aul;
    ctx->state[4] = 0x510e527ful;
    ctx->state[5] = 0x9b05688cul;
    ctx->state[6] = 0x1f83d9abul;
    ctx->state[7] = 0x5be0cd19ul;
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
    ctx->bitlen += (uint64_t)ctx->datalen * 8ull;
    ctx->data[63] = (uint8_t)(ctx->bitlen);
    ctx->data[62] = (uint8_t)(ctx->bitlen >> 8);
    ctx->data[61] = (uint8_t)(ctx->bitlen >> 16);
    ctx->data[60] = (uint8_t)(ctx->bitlen >> 24);
    ctx->data[59] = (uint8_t)(ctx->bitlen >> 32);
    ctx->data[58] = (uint8_t)(ctx->bitlen >> 40);
    ctx->data[57] = (uint8_t)(ctx->bitlen >> 48);
    ctx->data[56] = (uint8_t)(ctx->bitlen >> 56);
    sha256_transform(ctx, ctx->data);
    for (i=0; i<4; ++i) {
        hash[i]      = (uint8_t)((ctx->state[0] >> (24 - i*8)) & 0xff);
        hash[i + 4]  = (uint8_t)((ctx->state[1] >> (24 - i*8)) & 0xff);
        hash[i + 8]  = (uint8_t)((ctx->state[2] >> (24 - i*8)) & 0xff);
        hash[i + 12] = (uint8_t)((ctx->state[3] >> (24 - i*8)) & 0xff);
        hash[i + 16] = (uint8_t)((ctx->state[4] >> (24 - i*8)) & 0xff);
        hash[i + 20] = (uint8_t)((ctx->state[5] >> (24 - i*8)) & 0xff);
        hash[i + 24] = (uint8_t)((ctx->state[6] >> (24 - i*8)) & 0xff);
        hash[i + 28] = (uint8_t)((ctx->state[7] >> (24 - i*8)) & 0xff);
    }
}

static std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data.data(), data.size());
    std::vector<uint8_t> out(32);
    sha256_final(&ctx, out.data());
    return out;
}

// --- Utilities ---

static bool constant_time_equals(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    size_t len = a.size() > b.size() ? a.size() : b.size();
    uint8_t diff = 0;
    for (size_t i=0; i<len; ++i) {
        uint8_t va = i < a.size() ? a[i] : 0;
        uint8_t vb = i < b.size() ? b[i] : 0;
        diff |= (uint8_t)(va ^ vb);
    }
    return diff == 0 && a.size() == b.size();
}

static std::string to_hex(const std::vector<uint8_t>& v) {
    std::ostringstream oss;
    for (auto b : v) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

static std::vector<uint8_t> hash_password(const std::vector<uint8_t>& salt, const std::string& password) {
    std::vector<uint8_t> input;
    input.reserve(salt.size()*2 + password.size());
    input.insert(input.end(), salt.begin(), salt.end());
    input.insert(input.end(), password.begin(), password.end());
    input.insert(input.end(), salt.begin(), salt.end());
    std::vector<uint8_t> out = sha256(input);
    for (int i=0; i<999; ++i) {
        std::vector<uint8_t> concat = out;
        concat.insert(concat.end(), salt.begin(), salt.end());
        out = sha256(concat);
    }
    return out;
}

static bool is_valid_email(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;
    static const std::regex re("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,189}\\.[A-Za-z]{2,24}$");
    return std::regex_match(email, re);
}

// --- Data structures and service ---

struct User {
    std::string username;
    std::string email;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> password_hash;
};

class AuthService {
public:
    AuthService() : currentUser(nullptr) {}

    static User register_default_user(const std::string& username, const std::string& email, const std::string& password) {
        std::random_device rd;
        std::vector<uint8_t> salt(16);
        for (auto &b : salt) b = static_cast<uint8_t>(rd());
        std::vector<uint8_t> ph = hash_password(salt, password);
        return User{username, email, salt, ph};
    }

    bool login(User& user, const std::string& username, const std::string& password) {
        if (user.username != username) return false;
        auto computed = hash_password(user.salt, password);
        if (constant_time_equals(user.password_hash, computed)) {
            currentUser = &user;
            return true;
        }
        return false;
    }

    void logout() { currentUser = nullptr; }

    std::string change_email(const std::string& old_email, const std::string& new_email, const std::string& confirm_password) {
        if (!currentUser) return "Error: You must be logged in to change your email.";
        std::string oldE = old_email;
        std::string newE = new_email;
        if (!is_valid_email(oldE) || !is_valid_email(newE)) {
            return "Error: Invalid email format.";
        }
        if (currentUser->email != oldE) {
            return "Error: The provided old email does not match our records.";
        }
        // Case-insensitive compare to avoid trivial same-email bypass
        auto lower = [](std::string s){ for (auto &c : s) c = (char)std::tolower((unsigned char)c); return s; };
        if (lower(oldE) == lower(newE)) {
            return "Error: New email must be different from old email.";
        }
        auto computed = hash_password(currentUser->salt, confirm_password);
        if (!constant_time_equals(currentUser->password_hash, computed)) {
            return "Error: Invalid credentials.";
        }
        currentUser->email = newE;
        return std::string("Success: Email changed to ") + newE + ".";
    }

    static std::string user_snapshot(const User& u) {
        return "User(username=" + u.username + ", email=" + u.email + ", salt_hex=" + to_hex(u.salt) + ")";
    }

private:
    User* currentUser;
};

int main() {
    User user = AuthService::register_default_user("alice", "alice@old.com", "P@ssw0rd!");
    AuthService auth;

    bool login1 = auth.login(user, "alice", "P@ssw0rd!");
    std::string res1 = auth.change_email("alice@old.com", "alice@new.com", "P@ssw0rd!");
    std::cout << "T1 login=" << (login1 ? "true" : "false") << " -> " << res1 << " | " << AuthService::user_snapshot(user) << "\n";
    auth.logout();

    std::string res2 = auth.change_email("alice@new.com", "alice@newer.com", "P@ssw0rd!");
    std::cout << "T2 -> " << res2 << " | " << AuthService::user_snapshot(user) << "\n";

    bool login3 = auth.login(user, "alice", "P@ssw0rd!");
    std::string res3 = auth.change_email("alice@new.com", "alice@fresh.com", "WRONGPASS");
    std::cout << "T3 login=" << (login3 ? "true" : "false") << " -> " << res3 << " | " << AuthService::user_snapshot(user) << "\n";
    auth.logout();

    bool login4 = auth.login(user, "alice", "P@ssw0rd!");
    std::string res4 = auth.change_email("alice@new.com", "alice@new.com", "P@ssw0rd!");
    std::cout << "T4 login=" << (login4 ? "true" : "false") << " -> " << res4 << " | " << AuthService::user_snapshot(user) << "\n";
    auth.logout();

    bool login5 = auth.login(user, "alice", "P@ssw0rd!");
    std::string res5 = auth.change_email("wrong@old.com", "alice@final.com", "P@ssw0rd!");
    std::cout << "T5 login=" << (login5 ? "true" : "false") << " -> " << res5 << " | " << AuthService::user_snapshot(user) << "\n";
    auth.logout();

    return 0;
}