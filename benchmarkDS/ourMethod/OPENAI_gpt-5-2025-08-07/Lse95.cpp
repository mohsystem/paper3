#include <iostream>
#include <string>
#include <unordered_map>
#include <regex>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const int SALT_LEN = 16;     // bytes
static const int KEY_LEN  = 32;     // bytes
static const int ITERATIONS = 210000;

static std::unordered_map<std::string, std::string> DB; // username -> hex(hash)+hex(salt)

static bool is_valid_username(const std::string& username) {
    static const std::regex re("^[A-Za-z0-9_]{3,32}$");
    return std::regex_match(username, re);
}

static bool is_strong_password(const std::string& password) {
    if (password.size() < 12) return false;
    bool has_upper=false, has_lower=false, has_digit=false, has_special=false;
    const std::string specials = "!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\ ";
    for (unsigned char c : password) {
        if (std::isupper(c)) has_upper = true;
        else if (std::islower(c)) has_lower = true;
        else if (std::isdigit(c)) has_digit = true;
        else if (specials.find((char)c) != std::string::npos) has_special = true;
    }
    return has_upper && has_lower && has_digit && has_special;
}

static std::string to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(hex[(data[i] >> 4) & 0xF]);
        out.push_back(hex[data[i] & 0xF]);
    }
    return out;
}

static bool from_hex(const std::string& hex, std::vector<unsigned char>& out) {
    if (hex.size() % 2 != 0) return false;
    out.resize(hex.size() / 2);
    auto hexval = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (size_t i = 0; i < out.size(); ++i) {
        int hi = hexval(hex[2*i]);
        int lo = hexval(hex[2*i + 1]);
        if (hi < 0 || lo < 0) return false;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return true;
}

static bool pbkdf2_sha256(const std::string& password, const unsigned char* salt, size_t salt_len, int iterations, unsigned char* out_key, size_t out_len) {
    const unsigned char* pw = reinterpret_cast<const unsigned char*>(password.data());
    return PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(pw), (int)password.size(), salt, (int)salt_len, iterations, EVP_sha256(), (int)out_len, out_key) == 1;
}

bool register_user(const std::string& username, const std::string& password) {
    if (!is_valid_username(username) || !is_strong_password(password)) {
        return false;
    }
    if (DB.find(username) != DB.end()) {
        return false;
    }
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return false;
    }
    unsigned char key[KEY_LEN];
    if (!pbkdf2_sha256(password, salt, SALT_LEN, ITERATIONS, key, KEY_LEN)) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }
    std::string stored = to_hex(key, KEY_LEN) + to_hex(salt, SALT_LEN); // append salt to hash
    DB[username] = stored;
    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(key, sizeof(key));
    return true;
}

bool verify_login(const std::string& username, const std::string& password) {
    auto it = DB.find(username);
    if (it == DB.end()) return false;
    const std::string& stored = it->second;
    if (stored.size() < (size_t)(SALT_LEN * 2)) return false;

    std::string salt_hex = stored.substr(stored.size() - SALT_LEN * 2);
    std::string hash_hex = stored.substr(0, stored.size() - SALT_LEN * 2);

    std::vector<unsigned char> salt;
    std::vector<unsigned char> expected;
    if (!from_hex(salt_hex, salt) || !from_hex(hash_hex, expected) || expected.size() != KEY_LEN || salt.size() != SALT_LEN) {
        return false;
    }
    unsigned char key[KEY_LEN];
    if (!pbkdf2_sha256(password, salt.data(), salt.size(), ITERATIONS, key, KEY_LEN)) {
        OPENSSL_cleanse(key, sizeof(key));
        return false;
    }
    bool ok = (CRYPTO_memcmp(expected.data(), key, KEY_LEN) == 0);
    OPENSSL_cleanse(key, sizeof(key));
    return ok;
}

int main() {
    // 1) Successful registration
    std::cout << "Test1 register alice: " << (register_user("alice_01", "Str0ng!Passw0rd") ? "true" : "false") << "\n";
    // 2) Successful login
    std::cout << "Test2 login alice correct: " << (verify_login("alice_01", "Str0ng!Passw0rd") ? "true" : "false") << "\n";
    // 3) Failed login due to wrong password
    std::cout << "Test3 login alice wrong: " << (verify_login("alice_01", "WrongPassword!1") ? "true" : "false") << "\n";
    // 4) Duplicate registration should fail
    std::cout << "Test4 duplicate register alice: " << (register_user("alice_01", "An0ther!Passw0rd") ? "true" : "false") << "\n";
    // 5) Weak password should fail registration
    std::cout << "Test5 register weak bob: " << (register_user("bob", "weakpass") ? "true" : "false") << "\n";
    return 0;
}