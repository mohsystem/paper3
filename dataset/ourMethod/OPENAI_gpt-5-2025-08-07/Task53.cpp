#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <algorithm>

// Minimal SHA-256 implementation (public domain style)
namespace sha256 {
    typedef uint32_t u32;
    typedef uint64_t u64;
    struct Ctx {
        u32 state[8];
        u64 bitlen;
        unsigned char data[64];
        size_t datalen;
    };

    static inline u32 rotr(u32 x, u32 n) { return (x >> n) | (x << (32 - n)); }
    static inline u32 ch(u32 x, u32 y, u32 z) { return (x & y) ^ (~x & z); }
    static inline u32 maj(u32 x, u32 y, u32 z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline u32 ep0(u32 x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline u32 ep1(u32 x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline u32 sig0(u32 x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline u32 sig1(u32 x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    static const u32 K[64] = {
        0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
        0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
        0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
        0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
        0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
        0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
        0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
        0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
    };

    static void init(Ctx &ctx) {
        ctx.datalen = 0;
        ctx.bitlen = 0;
        ctx.state[0] = 0x6a09e667ul;
        ctx.state[1] = 0xbb67ae85ul;
        ctx.state[2] = 0x3c6ef372ul;
        ctx.state[3] = 0xa54ff53aul;
        ctx.state[4] = 0x510e527ful;
        ctx.state[5] = 0x9b05688cul;
        ctx.state[6] = 0x1f83d9abul;
        ctx.state[7] = 0x5be0cd19ul;
    }

    static void transform(Ctx &ctx, const unsigned char data[]) {
        u32 m[64];
        for (int i = 0; i < 16; ++i) {
            m[i] = (u32)data[i*4] << 24 | (u32)data[i*4+1] << 16 | (u32)data[i*4+2] << 8 | (u32)data[i*4+3];
        }
        for (int i = 16; i < 64; ++i) {
            m[i] = sig1(m[i-2]) + m[i-7] + sig0(m[i-15]) + m[i-16];
        }

        u32 a=ctx.state[0], b=ctx.state[1], c=ctx.state[2], d=ctx.state[3];
        u32 e=ctx.state[4], f=ctx.state[5], g=ctx.state[6], h=ctx.state[7];

        for (int i = 0; i < 64; ++i) {
            u32 t1 = h + ep1(e) + ch(e,f,g) + K[i] + m[i];
            u32 t2 = ep0(a) + maj(a,b,c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        ctx.state[0] += a;
        ctx.state[1] += b;
        ctx.state[2] += c;
        ctx.state[3] += d;
        ctx.state[4] += e;
        ctx.state[5] += f;
        ctx.state[6] += g;
        ctx.state[7] += h;
    }

    static void update(Ctx &ctx, const unsigned char *data, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            ctx.data[ctx.datalen] = data[i];
            ctx.datalen++;
            if (ctx.datalen == 64) {
                transform(ctx, ctx.data);
                ctx.bitlen += 512;
                ctx.datalen = 0;
            }
        }
    }

    static void final(Ctx &ctx, unsigned char hash[32]) {
        size_t i = ctx.datalen;
        // Pad
        if (ctx.datalen < 56) {
            ctx.data[i++] = 0x80;
            while (i < 56) ctx.data[i++] = 0x00;
        } else {
            ctx.data[i++] = 0x80;
            while (i < 64) ctx.data[i++] = 0x00;
            transform(ctx, ctx.data);
            std::fill(std::begin(ctx.data), std::end(ctx.data), 0);
        }
        ctx.bitlen += ctx.datalen * 8;
        // Append length big-endian
        for (int j = 7; j >= 0; --j) {
            ctx.data[56 + (7 - j)] = (unsigned char)((ctx.bitlen >> (j * 8)) & 0xFFu);
        }
        transform(ctx, ctx.data);
        // Output
        for (i = 0; i < 4; ++i) {
            hash[i]      = (ctx.state[0] >> (24 - i * 8)) & 0xFFu;
            hash[i + 4]  = (ctx.state[1] >> (24 - i * 8)) & 0xFFu;
            hash[i + 8]  = (ctx.state[2] >> (24 - i * 8)) & 0xFFu;
            hash[i + 12] = (ctx.state[3] >> (24 - i * 8)) & 0xFFu;
            hash[i + 16] = (ctx.state[4] >> (24 - i * 8)) & 0xFFu;
            hash[i + 20] = (ctx.state[5] >> (24 - i * 8)) & 0xFFu;
            hash[i + 24] = (ctx.state[6] >> (24 - i * 8)) & 0xFFu;
            hash[i + 28] = (ctx.state[7] >> (24 - i * 8)) & 0xFFu;
        }
    }

    static std::vector<unsigned char> hash(const std::vector<unsigned char>& data) {
        Ctx ctx;
        init(ctx);
        update(ctx, data.data(), data.size());
        std::vector<unsigned char> out(32);
        final(ctx, out.data());
        return out;
    }
}

// Security helpers
static bool constantTimeEqual(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    if (a.size() != b.size()) return false;
    unsigned char res = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        res |= (unsigned char)(a[i] ^ b[i]);
    }
    return res == 0;
}
static bool constantTimeEqualStr(const std::string& a, const std::string& b) {
    size_t la = a.size(), lb = b.size();
    size_t l = std::max(la, lb);
    unsigned char r = 0;
    for (size_t i = 0; i < l; ++i) {
        unsigned char ca = i < la ? (unsigned char)a[i] : 0;
        unsigned char cb = i < lb ? (unsigned char)b[i] : 0;
        r |= (unsigned char)(ca ^ cb);
    }
    return (la == lb) && (r == 0);
}

static bool secureRandomBytes(std::vector<unsigned char>& out) {
    // Try /dev/urandom
    std::ifstream ur("/dev/urandom", std::ios::in | std::ios::binary);
    if (ur) {
        ur.read(reinterpret_cast<char*>(out.data()), (std::streamsize)out.size());
        if (ur.good() || ur.gcount() == (std::streamsize)out.size()) {
            return true;
        }
    }
    // Fallback to std::random_device per-byte
    try {
        std::random_device rd;
        for (size_t i = 0; i < out.size(); ++i) {
            out[i] = static_cast<unsigned char>(rd());
        }
        return true;
    } catch (...) {
        return false;
    }
}

static std::string toHex(const std::vector<unsigned char>& data) {
    static const char* hex = "0123456789abcdef";
    std::string s;
    s.reserve(data.size() * 2);
    for (unsigned char b : data) {
        s.push_back(hex[b >> 4]);
        s.push_back(hex[b & 0x0F]);
    }
    return s;
}

static std::vector<unsigned char> concat(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    std::vector<unsigned char> r;
    r.reserve(a.size() + b.size());
    r.insert(r.end(), a.begin(), a.end());
    r.insert(r.end(), b.begin(), b.end());
    return r;
}

// Password policy
static bool isStrongPassword(const std::string& pw) {
    if (pw.size() < 12 || pw.size() > 128) return false;
    bool hasU=false, hasL=false, hasD=false, hasS=false;
    for (unsigned char c : pw) {
        if (c == ' ') return false;
        if (c >= 'A' && c <= 'Z') hasU = true;
        else if (c >= 'a' && c <= 'z') hasL = true;
        else if (c >= '0' && c <= '9') hasD = true;
        else hasS = true;
    }
    static const char* common[] = {
        "password", "12345678", "qwertyui", "letmein123", "adminadmin",
        "password1", "iloveyou", "welcome123", "monkey123", "abc123abc"
    };
    std::string lower = pw;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    for (const char* c : common) {
        if (lower.find(c) != std::string::npos) return false;
    }
    return hasU && hasL && hasD && hasS;
}

// Username validation
static bool isValidUsername(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) return false;
    for (unsigned char c : u) {
        if (!(std::isalnum(c) || c == '_' || c=='-' || c=='.')) return false;
    }
    return true;
}

// User and Session management
struct User {
    std::vector<unsigned char> salt; // 16 bytes
    std::vector<unsigned char> hash; // 32 bytes
    std::chrono::system_clock::time_point pw_set;
};

class UserStore {
public:
    bool registerUser(const std::string& username, const std::string& password) {
        if (!isValidUsername(username) || !isStrongPassword(password)) return false;
        if (users.find(username) != users.end()) return false;

        std::vector<unsigned char> salt(16);
        if (!secureRandomBytes(salt)) return false;
        std::vector<unsigned char> pwBytes(password.begin(), password.end());
        std::vector<unsigned char> salted = concat(salt, pwBytes);
        auto digest = sha256::hash(salted);

        User u;
        u.salt = salt;
        u.hash = digest;
        u.pw_set = std::chrono::system_clock::now();
        users.emplace(username, std::move(u));
        return true;
    }

    bool verifyPassword(const std::string& username, const std::string& password) const {
        auto it = users.find(username);
        if (it == users.end()) return false;
        const User& u = it->second;
        // Check password not expired (90 days)
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::hours>(now - u.pw_set).count();
        if (age > 90LL * 24LL) {
            return false;
        }
        std::vector<unsigned char> pwBytes(password.begin(), password.end());
        std::vector<unsigned char> salted = concat(u.salt, pwBytes);
        auto digest = sha256::hash(salted);
        return constantTimeEqual(digest, u.hash);
    }

private:
    std::unordered_map<std::string, User> users;
};

struct Session {
    std::string username;
    std::chrono::system_clock::time_point expiry;
};

class SessionManager {
public:
    SessionManager() : ttlSeconds(900) {}

    void setTTL(int seconds) {
        if (seconds > 0 && seconds <= 24*3600) {
            ttlSeconds = seconds;
        }
    }

    bool createSession(const std::string& username, std::string& outToken) {
        std::vector<unsigned char> tokenBytes(32);
        if (!secureRandomBytes(tokenBytes)) return false;
        std::string token = toHex(tokenBytes);
        Session s;
        s.username = username;
        s.expiry = std::chrono::system_clock::now() + std::chrono::seconds(ttlSeconds);
        sessions[token] = s;
        outToken = token;
        return true;
    }

    bool validate(const std::string& token, std::string& outUsername) {
        cleanupExpired();
        auto it = sessions.find(token);
        if (it == sessions.end()) return false;
        // constant-time lookup verification
        if (!constantTimeEqualStr(it->first, token)) return false;
        if (std::chrono::system_clock::now() > it->second.expiry) {
            sessions.erase(it);
            return false;
        }
        outUsername = it->second.username;
        return true;
    }

    bool logout(const std::string& token) {
        auto it = sessions.find(token);
        if (it == sessions.end()) return false;
        if (!constantTimeEqualStr(it->first, token)) return false;
        sessions.erase(it);
        return true;
    }

    void cleanupExpired() {
        auto now = std::chrono::system_clock::now();
        for (auto it = sessions.begin(); it != sessions.end(); ) {
            if (now > it->second.expiry) it = sessions.erase(it);
            else ++it;
        }
    }

private:
    std::unordered_map<std::string, Session> sessions;
    int ttlSeconds;
};

// Facade for the application
class AuthService {
public:
    bool registerUser(const std::string& username, const std::string& password) {
        return users.registerUser(username, password);
    }
    bool login(const std::string& username, const std::string& password, std::string& outToken) {
        if (!users.verifyPassword(username, password)) return false;
        return sessions.createSession(username, outToken);
    }
    bool validateSession(const std::string& token, std::string& outUsername) {
        return sessions.validate(token, outUsername);
    }
    bool logout(const std::string& token) {
        return sessions.logout(token);
    }
    void setSessionTTL(int seconds) {
        sessions.setTTL(seconds);
    }
    void cleanup() {
        sessions.cleanupExpired();
    }
private:
    UserStore users;
    SessionManager sessions;
};

static void printResult(const std::string& name, bool ok) {
    std::cout << name << ": " << (ok ? "PASS" : "FAIL") << "\n";
}

int main() {
    AuthService svc;

    // Test 1: Register and login success
    bool t1 = svc.registerUser("alice_user", "Str0ng!Passw0rd");
    std::string token1;
    bool t1b = svc.login("alice_user", "Str0ng!Passw0rd", token1);
    std::string uname1;
    bool t1c = svc.validateSession(token1, uname1);
    printResult("Test1_Register", t1);
    printResult("Test1_Login", t1b);
    printResult("Test1_Validate", t1c && uname1 == "alice_user");

    // Test 2: Duplicate registration
    bool t2 = !svc.registerUser("alice_user", "An0ther!GoodOne");
    printResult("Test2_DuplicateRegister", t2);

    // Test 3: Weak password rejected
    bool t3 = !svc.registerUser("bob", "weakpass"); // too weak
    printResult("Test3_WeakPasswordRejected", t3);

    // Test 4: Wrong password login fails
    bool t4a = svc.registerUser("charlie", "Anoth3r!StrongOne");
    std::string token4;
    bool t4b = !svc.login("charlie", "WrongPass!23", token4);
    printResult("Test4_RegisterCharlie", t4a);
    printResult("Test4_LoginWrongPassword", t4b);

    // Test 5: Session expiration
    bool t5a = svc.registerUser("diana", "Sup3r!Secur3PW");
    svc.setSessionTTL(1);
    std::string token5;
    bool t5b = svc.login("diana", "Sup3r!Secur3PW", token5);
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    std::string uname5;
    bool t5c = !svc.validateSession(token5, uname5);
    printResult("Test5_RegisterDiana", t5a);
    printResult("Test5_LoginDiana", t5b);
    printResult("Test5_SessionExpired", t5c);

    // Logout test (extra within 5th series): create new session and logout
    std::string token6;
    bool t6a = svc.login("alice_user", "Str0ng!Passw0rd", token6);
    bool t6b = svc.logout(token6);
    std::string uname6;
    bool t6c = !svc.validateSession(token6, uname6);
    printResult("Test5b_Logout", t6a && t6b && t6c);

    return 0;
}