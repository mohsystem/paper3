
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/sha.h>
#include <openssl/rand.h>

std::string base64_encode(const unsigned char* data, size_t length) {
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0;
    int valb = -6;
    
    for (size_t i = 0; i < length; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

std::string base64_decode(const std::string& input) {
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0;
    int valb = -8;
    
    for (unsigned char c : input) {
        if (c == '=') break;
        const char* p = strchr(base64_chars, c);
        if (p == nullptr) continue;
        val = (val << 6) + (p - base64_chars);
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

std::string generate_salt() {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    return base64_encode(salt, 16);
}

std::string hash_password(const std::string& password, const std::string& salt) {
    std::string decoded_salt = base64_decode(salt);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, decoded_salt.c_str(), decoded_salt.length());
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

std::string register_user(const std::string& username, const std::string& password) {
    std::string salt = generate_salt();
    std::string hashed_password = hash_password(password, salt);
    std::string stored_value = salt + ":" + hashed_password;
    return stored_value;
}

int main() {
    // Test case 1
    std::string result1 = register_user("user1", "password123");
    std::cout << "Test 1 - User: user1, Stored: " << result1 << std::endl;
    
    // Test case 2
    std::string result2 = register_user("alice", "securePass456");
    std::cout << "Test 2 - User: alice, Stored: " << result2 << std::endl;
    
    // Test case 3
    std::string result3 = register_user("bob", "myP@ssw0rd");
    std::cout << "Test 3 - User: bob, Stored: " << result3 << std::endl;
    
    // Test case 4
    std::string result4 = register_user("admin", "Admin#2024");
    std::cout << "Test 4 - User: admin, Stored: " << result4 << std::endl;
    
    // Test case 5
    std::string result5 = register_user("testuser", "Test!ng123");
    std::cout << "Test 5 - User: testuser, Stored: " << result5 << std::endl;
    
    return 0;
}
