#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <ctime>
#include <cctype>

static const int ITERATIONS = 210000;
static const size_t SALT_LEN = 16;
static const size_t KEY_LEN = 32;
static const size_t TOKEN_LEN = 32;
static const int TOKEN_TTL = 15 * 60; // seconds

struct UserRecord {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> pwd_hash;
    std::time_t pwd_changed_at;

    std::vector<unsigned char> token_salt;
    std::vector<unsigned char> token_hash;
    std::time_t token_expiry;
    bool has_token;

    UserRecord()
        : pwd_changed_at(0), token_expiry(0), has_token(false) {}
};

class PasswordManager {
public:
    bool registerUser(const std::string& username, const std::string& password) {
        if (!validateUsername(username) || !isStrongPassword(password, username)) return false;
        if (db.find(username) != db.end()) return false;

        std::vector<unsigned char> salt = randomBytes(SALT_LEN);
        std::vector<unsigned char> hash;
        if (!pbkdf2(reinterpret_cast<const unsigned char*>(password.data()), password.size(), salt, KEY_LEN, hash)) {
            return false;
        }
        UserRecord rec;
        rec.username = username;
        rec.salt = salt;
        rec.pwd_hash = hash;
        rec.pwd_changed_at = std::time(nullptr);
        rec.has_token = false;
        db[username] = std::move(rec);
        cleanse(hash);
        return true;
    }

    bool authenticate(const std::string& username, const std::string& password) {
        if (!validateUsername(username)) return false;
        auto it = db.find(username);
        if (it == db.end()) return false;
        UserRecord& u = it->second;
        std::vector<unsigned char> derived;
        if (!pbkdf2(reinterpret_cast<const unsigned char*>(password.data()), password.size(), u.salt, KEY_LEN, derived)) {
            return false;
        }
        bool ok = constantTimeEq(derived, u.pwd_hash);
        cleanse(derived);
        return ok;
    }

    std::string requestPasswordReset(const std::string& username) {
        if (!validateUsername(username)) return "";
        auto it = db.find(username);
        if (it == db.end()) {
            return "";
        }
        UserRecord& u = it->second;
        std::vector<unsigned char> token = randomBytes(TOKEN_LEN);
        std::vector<unsigned char> tokenSalt = randomBytes(SALT_LEN);
        std::vector<unsigned char> tokenHash;
        if (!pbkdf2(token.data(), token.size(), tokenSalt, KEY_LEN, tokenHash)) {
            return "";
        }
        u.token_salt = tokenSalt;
        u.token_hash = tokenHash;
        u.token_expiry = std::time(nullptr) + TOKEN_TTL;
        u.has_token = true;

        std::string tokenHex = toHex(token);
        cleanse(token);
        return tokenHex;
    }

    bool resetPassword(const std::string& username, const std::string& tokenHex, const std::string& newPassword) {
        if (!validateUsername(username)) return false;
        auto it = db.find(username);
        if (it == db.end()) return false;
        UserRecord& u = it->second;
        if (!u.has_token) return false;
        if (std::time(nullptr) > u.token_expiry) return false;

        std::vector<unsigned char> token;
        if (!fromHex(tokenHex, token) || token.size() != TOKEN_LEN) return false;

        std::vector<unsigned char> providedHash;
        if (!pbkdf2(token.data(), token.size(), u.token_salt, KEY_LEN, providedHash)) return false;
        bool tokenMatch = constantTimeEq(providedHash, u.token_hash);
        cleanse(providedHash);
        cleanse(token);
        if (!tokenMatch) return false;

        if (!isStrongPassword(newPassword, username)) return false;

        // Prevent password reuse
        std::vector<unsigned char> newWithOldSalt;
        if (!pbkdf2(reinterpret_cast<const unsigned char*>(newPassword.data()), newPassword.size(), u.salt, KEY_LEN, newWithOldSalt)) {
            return false;
        }
        bool sameAsOld = constantTimeEq(newWithOldSalt, u.pwd_hash);
        cleanse(newWithOldSalt);
        if (sameAsOld) return false;

        // Update password with fresh salt
        std::vector<unsigned char> newSalt = randomBytes(SALT_LEN);
        std::vector<unsigned char> newHash;
        if (!pbkdf2(reinterpret_cast<const unsigned char*>(newPassword.data()), newPassword.size(), newSalt, KEY_LEN, newHash)) {
            return false;
        }
        u.salt = newSalt;
        u.pwd_hash = newHash;
        u.pwd_changed_at = std::time(nullptr);
        u.token_salt.clear();
        u.token_hash.clear();
        u.token_expiry = 0;
        u.has_token = false;
        return true;
    }

private:
    std::unordered_map<std::string, UserRecord> db;

    static bool validateUsername(const std::string& username) {
        if (username.size() < 3 || username.size() > 32) return false;
        for (char c : username) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
        }
        return true;
    }

    static bool isStrongPassword(const std::string& password, const std::string& username) {
        if (password.size() < 12 || password.size() > 128) return false;
        bool up=false, lo=false, di=false, sp=false;
        for (char c : password) {
            if (std::isupper(static_cast<unsigned char>(c))) up = true;
            else if (std::islower(static_cast<unsigned char>(c))) lo = true;
            else if (std::isdigit(static_cast<unsigned char>(c))) di = true;
            else if (!std::isspace(static_cast<unsigned char>(c))) sp = true;
            if (std::isspace(static_cast<unsigned char>(c))) return false;
        }
        if (!(up && lo && di && sp)) return false;
        std::string lc = password;
        for (char& c : lc) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        std::string uLc = username;
        for (char& c : uLc) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (!uLc.empty() && lc.find(uLc) != std::string::npos) return false;
        const char* common[] = {"password","123456","qwerty","letmein","admin","welcome","iloveyou","monkey","abc123"};
        for (const char* w : common) {
            if (lc.find(w) != std::string::npos) return false;
        }
        return true;
    }

    static bool pbkdf2(const unsigned char* input, size_t input_len,
                       const std::vector<unsigned char>& salt,
                       size_t out_len, std::vector<unsigned char>& out) {
        out.assign(out_len, 0);
        if (PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(input), static_cast<int>(input_len),
                              salt.data(), static_cast<int>(salt.size()),
                              ITERATIONS, EVP_sha256(), static_cast<int>(out_len), out.data()) != 1) {
            return false;
        }
        return true;
    }

    static std::vector<unsigned char> randomBytes(size_t len) {
        std::vector<unsigned char> b(len);
        if (RAND_bytes(b.data(), static_cast<int>(len)) != 1) {
            // best-effort zero
            std::fill(b.begin(), b.end(), 0);
        }
        return b;
    }

    static void cleanse(std::vector<unsigned char>& v) {
        if (!v.empty()) OPENSSL_cleanse(v.data(), v.size());
    }

    static bool constantTimeEq(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) return false;
        return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
    }

    static std::string toHex(const std::vector<unsigned char>& data) {
        static const char* hex = "0123456789abcdef";
        std::string s;
        s.reserve(data.size() * 2);
        for (unsigned char c : data) {
            s.push_back(hex[(c >> 4) & 0xF]);
            s.push_back(hex[c & 0xF]);
        }
        return s;
    }

    static bool fromHex(const std::string& hex, std::vector<unsigned char>& out) {
        if (hex.size() % 2 != 0) return false;
        out.clear();
        out.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            int hi = hexDigit(hex[i]);
            int lo = hexDigit(hex[i+1]);
            if (hi < 0 || lo < 0) return false;
            out.push_back(static_cast<unsigned char>((hi << 4) | lo));
        }
        return true;
    }

    static int hexDigit(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }
};

int main() {
    PasswordManager pm;
    bool t1 = pm.registerUser("alice", "Str0ng!Passw0rd");
    bool t2 = pm.authenticate("alice", "Str0ng!Passw0rd");
    std::string token = pm.requestPasswordReset("alice");
    bool t3 = pm.resetPassword("alice", token, "An0ther!StrongPwd") && pm.authenticate("alice", "An0ther!StrongPwd");
    bool t4 = pm.resetPassword("alice", "deadbeef", "Yet@N0therStrong1");
    std::string token2 = pm.requestPasswordReset("alice");
    bool t5 = pm.resetPassword("alice", token2, "weakpass");
    std::cout << "Test1_RegisterStrongPassword: " << (t1 ? "true" : "false") << "\n";
    std::cout << "Test2_AuthenticateCorrect: " << (t2 ? "true" : "false") << "\n";
    std::cout << "Test3_ResetWithValidTokenAndAuth: " << (t3 ? "true" : "false") << "\n";
    std::cout << "Test4_ResetWithInvalidToken: " << (t4 ? "true" : "false") << "\n";
    std::cout << "Test5_AttemptWeakPasswordReset: " << (t5 ? "true" : "false") << "\n";
    return 0;
}