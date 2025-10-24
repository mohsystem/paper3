#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/sha.h>
#include <ctime>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <regex>

static const int SALT_LEN = 16;
static const int HASH_LEN = 32;
static const int ITERATIONS = 210000;
static const long PASSWORD_EXPIRATION_SECONDS = 90L * 24L * 60L * 60L; // 90 days

struct User {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
    std::time_t createdAt;
};

class UserStore {
public:
    std::string registerUser(const std::string& username, const std::string& password) {
        std::string uErr = validateUsername(username);
        if (!uErr.empty()) return "ERROR: " + uErr;
        if (users.find(username) != users.end()) return "ERROR: username already exists";
        std::string pErr = validatePasswordPolicy(username, password);
        if (!pErr.empty()) return "ERROR: " + pErr;

        std::vector<unsigned char> salt(SALT_LEN);
        if (RAND_bytes(salt.data(), SALT_LEN) != 1) return "ERROR: internal error";
        std::vector<unsigned char> hash(HASH_LEN);
        if (!pbkdf2(password, salt, ITERATIONS, hash)) return "ERROR: internal error";

        User u{username, salt, hash, ITERATIONS, std::time(nullptr)};
        users.emplace(username, std::move(u));
        return "OK";
    }

    bool authenticate(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it == users.end()) {
            simulateWork(password);
            return false;
        }
        if (isExpired(it->second)) {
            simulateWork(password);
            return false;
        }
        std::vector<unsigned char> test(it->second.hash.size());
        if (!pbkdf2(password, it->second.salt, it->second.iterations, test)) return false;
        if (test.size() != it->second.hash.size()) return false;
        return CRYPTO_memcmp(test.data(), it->second.hash.data(), (size_t)test.size()) == 0;
    }

private:
    std::unordered_map<std::string, User> users;

    static bool pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, int iterations, std::vector<unsigned char>& out) {
        out.resize(out.size());
        const unsigned char* saltPtr = salt.data();
        unsigned char* outPtr = out.data();
        int ok = PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                                   saltPtr, (int)salt.size(),
                                   iterations, EVP_sha256(), (int)out.size(), outPtr);
        return ok == 1;
    }

    static void simulateWork(const std::string& password) {
        std::vector<unsigned char> salt(SALT_LEN);
        RAND_bytes(salt.data(), SALT_LEN);
        std::vector<unsigned char> out(HASH_LEN);
        PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(),
                          salt.data(), (int)salt.size(),
                          ITERATIONS, EVP_sha256(), (int)out.size(), out.data());
    }

    static bool isExpired(const User& u) {
        std::time_t now = std::time(nullptr);
        return (now - u.createdAt) > PASSWORD_EXPIRATION_SECONDS;
    }

    static std::string validateUsername(const std::string& username) {
        if (username.size() < 3 || username.size() > 32) return "username length must be 3-32";
        static const std::regex re("^[A-Za-z0-9_-]+$");
        if (!std::regex_match(username, re)) return "username may contain letters, digits, '_' or '-'";
        return "";
    }

    static std::string validatePasswordPolicy(const std::string& username, const std::string& password) {
        if (password.size() < 12 || password.size() > 128) return "password length must be 12-128";
        bool hasLower=false, hasUpper=false, hasDigit=false, hasSpecial=false;
        for (unsigned char c : password) {
            if (std::islower(c)) hasLower = true;
            else if (std::isupper(c)) hasUpper = true;
            else if (std::isdigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        if (!(hasLower && hasUpper && hasDigit && hasSpecial)) {
            return "password must contain upper, lower, digit, and special character";
        }
        std::string lowerPass = password;
        std::string lowerUser = username;
        for (auto& c : lowerPass) c = (char)std::tolower((unsigned char)c);
        for (auto& c : lowerUser) c = (char)std::tolower((unsigned char)c);
        if (lowerPass.find(lowerUser) != std::string::npos) return "password must not contain the username";

        static const std::vector<std::string> blacklist = {
            "password", "123456", "123456789", "qwerty", "letmein", "welcome", "admin", "iloveyou"
        };
        for (const auto& b : blacklist) {
            if (lowerPass == b) return "password too common";
        }
        return "";
    }
};

int main() {
    UserStore store;

    // Test 1: Register alice with strong password
    std::string t1 = store.registerUser("alice", "Str0ng!Passw0rd");
    std::cout << "Test1 register alice: " << t1 << "\n";

    // Test 2: Authenticate alice with correct password
    bool t2 = store.authenticate("alice", "Str0ng!Passw0rd");
    std::cout << "Test2 auth alice correct: " << (t2 ? "true" : "false") << "\n";

    // Test 3: Authenticate alice with wrong password
    bool t3 = store.authenticate("alice", "WrongPass!123");
    std::cout << "Test3 auth alice wrong: " << (t3 ? "true" : "false") << "\n";

    // Test 4: Register bob with weak password
    std::string t4 = store.registerUser("bob", "password");
    std::cout << "Test4 register bob weak: " << t4 << "\n";

    // Test 5: Register alice again (duplicate)
    std::string t5 = store.registerUser("alice", "An0ther$tr0ngOne");
    std::cout << "Test5 register alice duplicate: " << t5 << "\n";

    return 0;
}