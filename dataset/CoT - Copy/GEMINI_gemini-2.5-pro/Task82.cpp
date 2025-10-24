#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>

// Note: This code requires linking with OpenSSL's libcrypto.
// Example compilation: g++ your_file.cpp -o your_app -lcrypto

const int ITERATIONS = 100000;
const int KEY_LENGTH = 32; // In bytes (256 bits)

/**
 * Converts a byte array into a hexadecimal string.
 */
std::string toHexString(const unsigned char* data, int len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

/**
 * Computes the hash of a password combined with a salt using PBKDF2.
 */
std::string computeHash(const std::string& password, const std::string& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH);

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        reinterpret_cast<const unsigned char*>(salt.c_str()),
        salt.length(),
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        hash.data()
    );

    if (result == 1) {
        return toHexString(hash.data(), hash.size());
    } else {
        // In a real application, this error should be handled more robustly.
        return ""; // Return empty string on failure
    }
}

int main() {
    struct TestCase {
        std::string password;
        std::string salt;
    };

    TestCase testCases[] = {
        {"password123", "somesalt1"},
        {"P@$$w0rd!", "anothersalt2"},
        {"a_very_secure_password", "salty-salt-salt"},
        {"", "emptypassword"},
        {"test", ""}
    };
    
    std::cout << "CPP PBKDF2-HMAC-SHA256 Hashes:" << std::endl;
    for (const auto& testCase : testCases) {
        std::string hash = computeHash(testCase.password, testCase.salt);
        std::cout << "Password: \"" << testCase.password << "\", Salt: \"" << testCase.salt << "\"" << std::endl;
        std::cout << "Hash: " << hash << std::endl;
        std::cout << std::endl;
    }

    return 0;
}