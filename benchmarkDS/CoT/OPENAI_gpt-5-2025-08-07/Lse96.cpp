#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

static std::unordered_map<std::string, std::string> DATABASE;
static const int SALT_LEN = 16;
static const int ITERATIONS = 100000;
static const int KEY_LEN = 32; // 256 bits

std::vector<unsigned char> generate_salt(int length = SALT_LEN) {
    std::vector<unsigned char> salt(length);
    if (RAND_bytes(salt.data(), length) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    return salt;
}

std::vector<unsigned char> derive_key(const std::string& password, const std::vector<unsigned char>& salt,
                                      int iterations = ITERATIONS, int key_len = KEY_LEN) {
    std::vector<unsigned char> key(key_len);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), static_cast<int>(salt.size()),
                          iterations, EVP_sha256(), key_len, key.data()) != 1) {
        throw std::runtime_error("PBKDF2 failed");
    }
    return key;
}

std::string to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    for (unsigned char b : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return oss.str();
}

std::string register_user(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) return std::string();
    if (DATABASE.find(username) != DATABASE.end()) return std::string();
    auto salt = generate_salt();
    auto key = derive_key(password, salt);
    std::string stored = to_hex(salt) + ":" + to_hex(key);
    DATABASE[username] = stored;
    return stored;
}

int main() {
    try {
        // 5 test cases
        std::cout << "Register alice: " << register_user("alice", "Password1!") << std::endl;
        std::cout << "Register bob: " << register_user("bob", "S3cure#Pass") << std::endl;
        std::cout << "Register carol: " << register_user("carol", "anotherPass123") << std::endl;
        std::cout << "Register alice again (should fail): " << register_user("alice", "newPass!") << std::endl;
        std::cout << "Register dave: " << register_user("dave", "p@ssW0rd") << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}