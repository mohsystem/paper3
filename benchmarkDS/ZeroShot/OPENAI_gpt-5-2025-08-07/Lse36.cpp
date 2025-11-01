#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/sha.h>

struct UserRecord {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
    int iterations;
};

static const int ITERATIONS_CPP = 120000;
static const int KEY_LEN_CPP = 32;
static const size_t MAX_USERNAME_LEN = 64;
static const size_t MAX_PASSWORD_LEN = 128;

static std::unordered_map<std::string, UserRecord> db_cpp;

static std::vector<unsigned char> pbkdf2_sha256(const std::string &password,
                                                const std::vector<unsigned char> &salt,
                                                int iterations, int keyLen) {
    std::vector<unsigned char> out(keyLen);
    PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                      salt.data(), static_cast<int>(salt.size()),
                      iterations, EVP_sha256(), keyLen, out.data());
    return out;
}

static bool constant_time_equals(const std::vector<unsigned char> &a,
                                 const std::vector<unsigned char> &b) {
    size_t maxlen = (a.size() > b.size()) ? a.size() : b.size();
    unsigned int diff = 0;
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char x = (i < a.size()) ? a[i] : 0;
        unsigned char y = (i < b.size()) ? b[i] : 0;
        diff |= (unsigned int)(x ^ y);
    }
    return diff == 0 && a.size() == b.size();
}

static void add_user(const std::string &username, const std::string &password,
                     const std::vector<unsigned char> &salt, int iterations) {
    auto hash = pbkdf2_sha256(password, salt, iterations, KEY_LEN_CPP);
    db_cpp[username] = UserRecord{salt, hash, iterations};
}

static std::string login(const std::string &username, const std::string &password) {
    if (username.empty() || username.size() > MAX_USERNAME_LEN) return "";
    if (password.size() > MAX_PASSWORD_LEN) return "";

    auto it = db_cpp.find(username);
    if (it == db_cpp.end()) {
        // Dummy work to mitigate user enumeration
        static const std::vector<unsigned char> dummy_salt = {
            0xA3,0x51,0x5C,0x09,0x7E,0x22,0xA9,0x41,0x13,0xF1,0xB2,0x08,0xCD,0x44,0x9A,0x7F
        };
        auto d1 = pbkdf2_sha256(password, dummy_salt, ITERATIONS_CPP, KEY_LEN_CPP);
        auto d2 = pbkdf2_sha256(password, dummy_salt, ITERATIONS_CPP, KEY_LEN_CPP);
        (void)constant_time_equals(d1, d2);
        return "";
    }

    const UserRecord &rec = it->second;
    auto computed = pbkdf2_sha256(password, rec.salt, rec.iterations, KEY_LEN_CPP);
    if (constant_time_equals(computed, rec.hash)) {
        return "/home";
    }
    return "";
}

static void initialize_db_cpp() {
    add_user("alice", "correct horse battery staple",
             {0x10,0x2F,0x3A,0x4B,0x5C,0x6D,0x7E,0x8F,0x90,0xAB,0xBC,0xCD,0xDE,0xEF,0x01,0x23},
             ITERATIONS_CPP);
    add_user("bob", "Tr0ub4dor&3",
             {0x33,0x55,0x77,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x12,0x34,0x56,0x78,0x9A,0xBC},
             ITERATIONS_CPP);
}

int main() {
    initialize_db_cpp();
    struct Test { std::string label, user, pwd; };
    std::vector<Test> tests = {
        {"Test1 - correct credentials", "alice", "correct horse battery staple"},
        {"Test2 - wrong password", "alice", "wrong password"},
        {"Test3 - unknown user", "charlie", "anything"},
        {"Test4 - empty username", "", "any"},
        {"Test5 - injection-like username", "alice' OR '1'='1", "foo"},
    };
    for (const auto &t : tests) {
        auto res = login(t.user, t.pwd);
        std::cout << t.label << ": " << (res == "/home" ? "redirect:/home" : "no-redirect") << "\n";
    }
    return 0;
}