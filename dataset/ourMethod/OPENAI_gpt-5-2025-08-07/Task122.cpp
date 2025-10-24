#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cctype>

struct UserRecord {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
};

class UserStore {
public:
    UserStore() = default;

    bool signup(const std::string& username, const std::string& password) {
        if (!validUsername(username)) return false;
        std::string reason;
        if (!validatePasswordPolicy(password, reason)) return false;
        if (users.find(username) != users.end()) return false;

        std::vector<unsigned char> salt(SALT_LEN);
        if (RAND_bytes(salt.data(), SALT_LEN) != 1) return false;

        std::vector<unsigned char> dk(DK_LEN);
        if (!pbkdf2(password, salt, ITERATIONS, dk)) return false;

        users.emplace(username, UserRecord{salt, dk, ITERATIONS});
        return true;
    }

private:
    static constexpr int SALT_LEN = 16;
    static constexpr int ITERATIONS = 210000;
    static constexpr int DK_LEN = 32;

    std::unordered_map<std::string, UserRecord> users;

    static bool validUsername(const std::string& u) {
        if (u.empty() || u.size() > 64) return false;
        for (char c : u) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    static bool validatePasswordPolicy(const std::string& p, std::string& reason) {
        if (p.size() < 12 || p.size() > 128) { reason = "length"; return false; }
        bool hasU=false, hasL=false, hasD=false, hasS=false;
        const std::string specials = "!@#$%^&*()-_=+[]{};:'\",.<>/?`~\\|";
        for (unsigned char uc : p) {
            if (uc == 0 || uc < 32) { reason = "invalid-char"; return false; }
            char c = static_cast<char>(uc);
            if (std::isupper(static_cast<unsigned char>(c))) hasU = true;
            else if (std::islower(static_cast<unsigned char>(c))) hasL = true;
            else if (std::isdigit(static_cast<unsigned char>(c))) hasD = true;
            else if (specials.find(c) != std::string::npos) hasS = true;
        }
        if (!(hasU && hasL && hasD && hasS)) { reason = "complexity"; return false; }
        reason = "ok"; return true;
    }

    static bool pbkdf2(const std::string& password,
                       const std::vector<unsigned char>& salt,
                       int iterations,
                       std::vector<unsigned char>& outKey) {
        if (PKCS5_PBKDF2_HMAC(password.c_str(),
                               static_cast<int>(password.size()),
                               salt.data(),
                               static_cast<int>(salt.size()),
                               iterations,
                               EVP_sha256(),
                               static_cast<int>(outKey.size()),
                               outKey.data()) != 1) {
            return false;
        }
        return true;
    }
};

int main() {
    UserStore store;

    struct Test { std::string u; std::string p; };
    std::vector<Test> tests = {
        {"alice", "ValidPassw0rd!"},   // success
        {"bob", "short"},              // fail: weak
        {"alice", "AnotherValid1!"},   // fail: duplicate
        {"carol", "Another$trong1"},   // success
        {"dave", "NoSpecialChar1"}     // fail: missing special
    };

    for (const auto& t : tests) {
        bool res = store.signup(t.u, t.p);
        std::cout << t.u << ": " << (res ? "signup ok" : "signup failed") << "\n";
    }
    return 0;
}