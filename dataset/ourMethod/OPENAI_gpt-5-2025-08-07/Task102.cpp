#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

struct UserRecord {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
};

class UserDatabase {
public:
    bool createUser(const std::string& username, const std::string& password) {
        if (!isValidUsername(username) || !isStrongPassword(password) || records.find(username) != records.end()) {
            return false;
        }
        std::vector<unsigned char> salt(SALT_LEN);
        if (RAND_bytes(salt.data(), SALT_LEN) != 1) return false;
        std::vector<unsigned char> digest(HASH_LEN);
        if (!pbkdf2(password, salt, ITERATIONS, digest)) return false;
        records[username] = UserRecord{salt, digest, ITERATIONS};
        return true;
    }

    bool updatePassword(const std::string& username, const std::string& newPassword) {
        if (!isValidUsername(username) || !isStrongPassword(newPassword)) {
            return false;
        }
        auto it = records.find(username);
        if (it == records.end()) return false;
        std::vector<unsigned char> salt(SALT_LEN);
        if (RAND_bytes(salt.data(), SALT_LEN) != 1) return false;
        std::vector<unsigned char> digest(HASH_LEN);
        if (!pbkdf2(newPassword, salt, ITERATIONS, digest)) return false;
        it->second = UserRecord{salt, digest, ITERATIONS};
        return true;
    }

    bool verify(const std::string& username, const std::string& password) const {
        if (!isValidUsername(username)) return false;
        auto it = records.find(username);
        if (it == records.end()) return false;
        std::vector<unsigned char> digest(HASH_LEN);
        if (!pbkdf2(password, it->second.salt, it->second.iterations, digest)) return false;
        if (digest.size() != it->second.hash.size()) return false;
        return CRYPTO_memcmp(digest.data(), it->second.hash.data(), digest.size()) == 0;
    }

private:
    static constexpr int SALT_LEN = 16;
    static constexpr int HASH_LEN = 32;
    static constexpr int ITERATIONS = 210000;

    std::unordered_map<std::string, UserRecord> records;

    static bool isValidUsername(const std::string& username) {
        if (username.size() < 3 || username.size() > 32) return false;
        for (char c : username) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
        }
        return true;
    }

    static bool isStrongPassword(const std::string& pwd) {
        if (pwd.size() < 12 || pwd.size() > 128) return false;
        bool hasL=false, hasU=false, hasD=false, hasS=false, noWS=true;
        for (unsigned char c : pwd) {
            if (std::isspace(c)) { noWS = false; break; }
            if (std::islower(c)) hasL = true;
            else if (std::isupper(c)) hasU = true;
            else if (std::isdigit(c)) hasD = true;
            else hasS = true;
        }
        return hasL && hasU && hasD && hasS && noWS;
    }

    static bool pbkdf2(const std::string& password, const std::vector<unsigned char>& salt, int iterations, std::vector<unsigned char>& out) {
        if (PKCS5_PBKDF2_HMAC(password.c_str(),
                              static_cast<int>(password.size()),
                              salt.data(),
                              static_cast<int>(salt.size()),
                              iterations,
                              EVP_sha256(),
                              static_cast<int>(out.size()),
                              out.data()) != 1) {
            return false;
        }
        return true;
    }
};

int main() {
    OpenSSL_add_all_algorithms();
    UserDatabase db;

    bool c1 = db.createUser("alice", "Start#Pass1234");
    bool c2 = db.createUser("bob_01", "Init$Secure5678");
    bool c3 = db.createUser("charlie_2", "My$trongPassw0rd!");
    bool c4 = db.createUser("dora", "DoraThe#Expl0rer");
    bool c5 = db.createUser("eve_user", "S3cure&EveUser!");
    std::cout << "Create users: " << (c1 && c2 && c3 && c4 && c5) << std::endl;

    bool t1u = db.updatePassword("alice", "NewStrong#Pass1234");
    bool t1v = db.verify("alice", "NewStrong#Pass1234");
    std::cout << "Test1 update+verify: " << (t1u && t1v) << std::endl;

    bool t2u = db.updatePassword("alice", "short");
    std::cout << "Test2 weak password rejected: " << (!t2u) << std::endl;

    bool t3u = db.updatePassword("unknown_user", "Valid#Password123");
    std::cout << "Test3 non-existing user: " << (!t3u) << std::endl;

    bool t4u = db.updatePassword("bob_01", "Another$trongP4ss!");
    bool t4v = db.verify("bob_01", "WrongPassword!234");
    std::cout << "Test4 update ok, wrong verify fails: " << (t4u && !t4v) << std::endl;

    std::string longPass = std::string("A1!") + std::string(20, '\0');
    longPass.clear();
    longPass = "A1!";
    for (int i = 0; i < 20; ++i) longPass += "xY9#";
    longPass += "Zz@2";
    bool t5u = db.updatePassword("charlie_2", longPass);
    bool t5v = db.verify("charlie_2", longPass);
    std::cout << "Test5 long password update+verify: " << (t5u && t5v) << std::endl;

    return 0;
}