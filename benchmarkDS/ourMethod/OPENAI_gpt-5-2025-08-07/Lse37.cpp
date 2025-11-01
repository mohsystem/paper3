#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <algorithm>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

struct UserRecord {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
};

class AuthService {
public:
    AuthService() : usernameRe("^[A-Za-z0-9_.@-]{1,64}$") {}

    bool registerUser(const std::string& username, const std::string& password) {
        if (!validateUsername(username) || !validatePassword(password)) {
            return false;
        }
        if (db.find(username) != db.end()) return false;

        std::vector<unsigned char> salt(16);
        if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
            return false;
        }

        std::vector<unsigned char> hash;
        if (!deriveKey(password, salt, iterations, dkLen, hash)) {
            return false;
        }
        db[username] = UserRecord{salt, hash, iterations};
        return true;
    }

    std::string authenticate(const std::string& username, const std::string& password) {
        try {
            if (!validateUsername(username) || password.size() < 1 || password.size() > maxPw) {
                return "cant_login";
            }
            auto it = db.find(username);
            if (it == db.end()) return "cant_login";
            const UserRecord& rec = it->second;
            if (rec.salt.empty() || rec.hash.empty() || rec.iterations < 1) {
                return "cant_login";
            }
            std::vector<unsigned char> cand;
            if (!deriveKey(password, rec.salt, rec.iterations, static_cast<int>(rec.hash.size()), cand)) {
                return "internal_server_error";
            }
            if (cand.size() != rec.hash.size()) return "internal_server_error";
            int diff = CRYPTO_memcmp(cand.data(), rec.hash.data(), rec.hash.size());
            std::fill(cand.begin(), cand.end(), 0);
            return (diff == 0) ? "success" : "cant_login";
        } catch (...) {
            return "internal_server_error";
        }
    }

    void corruptUserRecord(const std::string& username) {
        auto it = db.find(username);
        if (it != db.end()) {
            it->second.salt.clear(); // cause validation failure -> cant_login or error path
        }
    }

private:
    std::unordered_map<std::string, UserRecord> db;
    const std::regex usernameRe;
    const int iterations = 210000;
    const int dkLen = 32;
    const size_t minPw = 8;
    const size_t maxPw = 128;

    bool validateUsername(const std::string& u) const {
        if (u.empty() || u.size() > 64) return false;
        return std::regex_match(u, usernameRe);
    }

    bool validatePassword(const std::string& p) const {
        if (p.size() < minPw || p.size() > maxPw) return false;
        bool hasUpper=false, hasLower=false, hasDigit=false, hasSpecial=false;
        std::string specials = "!@#$%^&*()-_=+[]{};:,.?/~";
        for (unsigned char c : p) {
            if (std::isupper(c)) hasUpper = true;
            else if (std::islower(c)) hasLower = true;
            else if (std::isdigit(c)) hasDigit = true;
            if (specials.find(static_cast<char>(c)) != std::string::npos) hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    bool deriveKey(const std::string& password,
                   const std::vector<unsigned char>& salt,
                   int iters, int outLen,
                   std::vector<unsigned char>& out) const
    {
        out.assign(outLen, 0);
        const EVP_MD* md = EVP_sha256();
        if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                              salt.data(), static_cast<int>(salt.size()),
                              iters, md, outLen, out.data()) != 1) {
            return false;
        }
        return true;
    }
};

static std::string generateStrongPassword(size_t length = 12) {
    if (length < 12) length = 12;
    const std::string U = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string L = "abcdefghijklmnopqrstuvwxyz";
    const std::string D = "0123456789";
    const std::string S = "!@#$%^&*()-_=+[]{};:,.?/~";
    const std::string ALL = U + L + D + S;

    std::string pwd;
    pwd.reserve(length);
    pwd.push_back(U[RAND_bytes_wrap(U.size())]);
    pwd.push_back(L[RAND_bytes_wrap(L.size())]);
    pwd.push_back(D[RAND_bytes_wrap(D.size())]);
    pwd.push_back(S[RAND_bytes_wrap(S.size())]);
    while (pwd.size() < length) {
        pwd.push_back(ALL[RAND_bytes_wrap(ALL.size())]);
    }
    // shuffle
    for (size_t i = pwd.size() - 1; i > 0; --i) {
        size_t j = RAND_bytes_wrap(i + 1);
        std::swap(pwd[i], pwd[j]);
    }
    return pwd;
}

static size_t RAND_bytes_wrap(size_t bound) {
    if (bound <= 1) return 0;
    unsigned int val = 0;
    unsigned char buf[4];
    if (RAND_bytes(buf, 4) != 1) {
        // fallback (should not happen)
        return 0;
    }
    val = (static_cast<unsigned int>(buf[0]) << 24) |
          (static_cast<unsigned int>(buf[1]) << 16) |
          (static_cast<unsigned int>(buf[2]) << 8)  |
          (static_cast<unsigned int>(buf[3]));
    return static_cast<size_t>(val % bound);
}

int main() {
    AuthService auth;
    std::string user1 = "alice@example.com";
    std::string user2 = "bob_user-1";
    std::string pass1 = generateStrongPassword(14);
    std::string pass2 = generateStrongPassword(16);

    bool r1 = auth.registerUser(user1, pass1);
    bool r2 = auth.registerUser(user2, pass2);

    // Test 1: correct login
    std::string t1 = auth.authenticate(user1, pass1);
    // Test 2: wrong password (mutate first char)
    std::string wrong = pass1;
    wrong[0] = (wrong[0] == 'A') ? 'B' : 'A';
    std::string t2 = auth.authenticate(user1, wrong);
    // Test 3: non-existent user
    std::string t3 = auth.authenticate("charlie", generateStrongPassword(12));
    // Test 4: invalid username format
    std::string t4 = auth.authenticate("invalid user name with spaces", generateStrongPassword(12));
    // Test 5: simulate error or invalid record
    auth.corruptUserRecord(user2);
    std::string t5 = auth.authenticate(user2, pass2);

    std::cout << "Test1: " << (r1 ? t1 : "internal_server_error") << "\n";
    std::cout << "Test2: " << t2 << "\n";
    std::cout << "Test3: " << t3 << "\n";
    std::cout << "Test4: " << t4 << "\n";
    std::cout << "Test5: " << (r2 ? t5 : "internal_server_error") << "\n";
    return 0;
}