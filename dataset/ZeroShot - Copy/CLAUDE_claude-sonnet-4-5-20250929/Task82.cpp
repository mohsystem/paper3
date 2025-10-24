
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

std::string hashPasswordWithSalt(const std::string& password, const std::string& salt) {
    std::string combined = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), 
           combined.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

int main() {
    // Test case 1
    std::string hash1 = hashPasswordWithSalt("password123", "randomsalt1");
    std::cout << "Test 1: " << hash1 << std::endl;
    
    // Test case 2
    std::string hash2 = hashPasswordWithSalt("securePass!", "salt2024");
    std::cout << "Test 2: " << hash2 << std::endl;
    
    // Test case 3
    std::string hash3 = hashPasswordWithSalt("admin", "xyz789");
    std::cout << "Test 3: " << hash3 << std::endl;
    
    // Test case 4
    std::string hash4 = hashPasswordWithSalt("MyP@ssw0rd", "abc123def");
    std::cout << "Test 4: " << hash4 << std::endl;
    
    // Test case 5
    std::string hash5 = hashPasswordWithSalt("test", "salt");
    std::cout << "Test 5: " << hash5 << std::endl;
    
    return 0;
}
