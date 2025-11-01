#include <bits/stdc++.h>
using namespace std;

// Minimal SHA-256 implementation (public domain-like, adapted for single-file use)
namespace sha256_impl {
    typedef uint32_t u32;
    typedef uint64_t u64;
    static inline u32 rotr(u32 x, u32 n) { return (x >> n) | (x << (32 - n)); }
    static inline u32 ch(u32 x, u32 y, u32 z) { return (x & y) ^ (~x & z); }
    static inline u32 maj(u32 x, u32 y, u32 z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline u32 big_sigma0(u32 x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline u32 big_sigma1(u32 x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline u32 small_sigma0(u32 x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline u32 small_sigma1(u32 x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

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

    vector<uint8_t> sha256(const vector<uint8_t>& data) {
        u32 h0 = 0x6a09e667ul, h1 = 0xbb67ae85ul, h2 = 0x3c6ef372ul, h3 = 0xa54ff53aul;
        h0 ^= 0x00000000ul; h1 ^= 0x00000000ul; h2 ^= 0x00000000ul; h3 ^= 0x00000000ul;
        u32 h4 = 0x510e527ful, h5 = 0x9b05688cul, h6 = 0x1f83d9abul, h7 = 0x5be0cd19ul;

        vector<uint8_t> padded = data;
        u64 bitlen = (u64)data.size() * 8ull;
        padded.push_back(0x80);
        while ((padded.size() % 64) != 56) padded.push_back(0x00);
        for (int i = 7; i >= 0; --i) padded.push_back((uint8_t)((bitlen >> (8 * i)) & 0xff));

        for (size_t chunk = 0; chunk < padded.size(); chunk += 64) {
            u32 w[64];
            for (int i = 0; i < 16; ++i) {
                size_t idx = chunk + i * 4;
                w[i] = (padded[idx] << 24) | (padded[idx + 1] << 16) | (padded[idx + 2] << 8) | (padded[idx + 3]);
            }
            for (int i = 16; i < 64; ++i) {
                w[i] = small_sigma1(w[i - 2]) + w[i - 7] + small_sigma0(w[i - 15]) + w[i - 16];
            }
            u32 a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;
            for (int i = 0; i < 64; ++i) {
                u32 t1 = h + big_sigma1(e) + ch(e, f, g) + K[i] + w[i];
                u32 t2 = big_sigma0(a) + maj(a, b, c);
                h = g; g = f; f = e; e = d + t1;
                d = c; c = b; b = a; a = t1 + t2;
            }
            h0 += a; h1 += b; h2 += c; h3 += d; h4 += e; h5 += f; h6 += g; h7 += h;
        }
        vector<uint8_t> out(32);
        u32 hs[8] = {h0, h1, h2, h3, h4, h5, h6, h7};
        for (int i = 0; i < 8; ++i) {
            out[i * 4 + 0] = (uint8_t)((hs[i] >> 24) & 0xff);
            out[i * 4 + 1] = (uint8_t)((hs[i] >> 16) & 0xff);
            out[i * 4 + 2] = (uint8_t)((hs[i] >> 8) & 0xff);
            out[i * 4 + 3] = (uint8_t)(hs[i] & 0xff);
        }
        return out;
    }
}

static bool constantTimeEquals(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    if (a.size() != b.size()) return false;
    unsigned char r = 0;
    for (size_t i = 0; i < a.size(); ++i) r |= (a[i] ^ b[i]);
    return r == 0;
}

static bool validEmail(const string& email) {
    if (email.empty() || email.size() > 254) return false;
    static const regex re("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    return regex_match(email, re);
}

static vector<uint8_t> hashPassword(const string& password, const vector<uint8_t>& salt) {
    vector<uint8_t> buf = salt;
    buf.insert(buf.end(), password.begin(), password.end());
    return sha256_impl::sha256(buf);
}

static vector<uint8_t> randomBytes(size_t n) {
    vector<uint8_t> out(n);
    ifstream ur("/dev/urandom", ios::in | ios::binary);
    if (ur.good()) {
        ur.read((char*)out.data(), n);
        if ((size_t)ur.gcount() == n) return out;
    }
    random_device rd;
    for (size_t i = 0; i < n; ++i) out[i] = static_cast<uint8_t>(rd());
    return out;
}

static string toHex(const vector<uint8_t>& v) {
    static const char* hex = "0123456789abcdef";
    string s; s.reserve(v.size() * 2);
    for (auto b : v) { s.push_back(hex[b >> 4]); s.push_back(hex[b & 0xF]); }
    return s;
}

struct User {
    string email;
    vector<uint8_t> salt;
    vector<uint8_t> pwHash;
};

class AuthSystem {
    unordered_map<string, User> users; // lower email -> User
    unordered_map<string, string> sessions; // token -> lower email
public:
    bool registerUser(const string& email, const string& password) {
        if (!validEmail(email) || password.empty()) return false;
        string lower = email;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (users.find(lower) != users.end()) return false;
        vector<uint8_t> salt = randomBytes(16);
        vector<uint8_t> h = hashPassword(password, salt);
        users[lower] = User{email, salt, h};
        return true;
    }

    string login(const string& email, const string& password) {
        string lower = email;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        auto it = users.find(lower);
        if (it == users.end()) return "";
        vector<uint8_t> h = hashPassword(password, it->second.salt);
        if (!constantTimeEquals(h, it->second.pwHash)) return "";
        string token = toHex(randomBytes(24));
        sessions[token] = lower;
        return token;
    }

    bool changeEmail(const string& sessionToken, const string& oldEmail, const string& newEmail, const string& confirmPassword) {
        if (sessionToken.empty() || oldEmail.empty() || newEmail.empty()) return false;
        if (!validEmail(newEmail)) return false;
        auto sit = sessions.find(sessionToken);
        if (sit == sessions.end()) return false;
        string lowerOld = oldEmail;
        transform(lowerOld.begin(), lowerOld.end(), lowerOld.begin(), ::tolower);
        if (sit->second != lowerOld) return false;
        auto it = users.find(lowerOld);
        if (it == users.end()) return false;
        string lowerNew = newEmail;
        transform(lowerNew.begin(), lowerNew.end(), lowerNew.begin(), ::tolower);
        if (users.find(lowerNew) != users.end()) return false;
        vector<uint8_t> h = hashPassword(confirmPassword, it->second.salt);
        if (!constantTimeEquals(h, it->second.pwHash)) return false;
        // Update user
        User u = it->second;
        users.erase(it);
        u.email = newEmail;
        users[lowerNew] = u;
        sessions[sessionToken] = lowerNew;
        return true;
    }
};

// 5 test cases in main
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    AuthSystem auth;
    cout << "Register Alice: " << (auth.registerUser("alice@example.com", "CorrectHorseBatteryStaple!") ? "true" : "false") << "\n";
    cout << "Register Bob: " << (auth.registerUser("bob@example.com", "B0bStrong#Pass") ? "true" : "false") << "\n";
    string token = auth.login("alice@example.com", "CorrectHorseBatteryStaple!");
    cout << "Alice token present: " << (!token.empty() ? "true" : "false") << "\n";

    bool t1 = auth.changeEmail("badtoken", "alice@example.com", "alice_new@example.com", "CorrectHorseBatteryStaple!");
    cout << "Test1 (invalid session): " << (t1 ? "true" : "false") << "\n";

    bool t2 = auth.changeEmail(token, "wrong@example.com", "alice_new@example.com", "CorrectHorseBatteryStaple!");
    cout << "Test2 (wrong old email): " << (t2 ? "true" : "false") << "\n";

    bool t3 = auth.changeEmail(token, "alice@example.com", "alice_new@example.com", "wrongpass");
    cout << "Test3 (wrong password): " << (t3 ? "true" : "false") << "\n";

    bool t4 = auth.changeEmail(token, "alice@example.com", "bob@example.com", "CorrectHorseBatteryStaple!");
    cout << "Test4 (email exists): " << (t4 ? "true" : "false") << "\n";

    bool t5 = auth.changeEmail(token, "alice@example.com", "alice2@example.com", "CorrectHorseBatteryStaple!");
    cout << "Test5 (success): " << (t5 ? "true" : "false") << "\n";

    // Verify login with new email
    string token2 = auth.login("alice2@example.com", "CorrectHorseBatteryStaple!");
    cout << "Login with new email works: " << (!token2.empty() ? "true" : "false") << "\n";

    return 0;
}