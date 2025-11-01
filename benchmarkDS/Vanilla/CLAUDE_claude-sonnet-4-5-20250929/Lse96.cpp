
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/rand.h>

std::string base64_encode(const unsigned char* buffer, size_t length) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (length--) {
        char_array_3[i++] = *(buffer++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];

        while(i++ < 3)
            result += '=';
    }

    return result;
}

std::string generateSalt() {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    return base64_encode(salt, 16);
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    // Decode base64 salt (simplified - assumes valid base64)
    SHA256_Update(&sha256, salt.c_str(), salt.length());
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

std::string registerUser(const std::string& username, const std::string& password) {
    std::string salt = generateSalt();
    std::string hashedPassword = hashPassword(password, salt);
    return salt + ":" + hashedPassword;
}

int main() {
    // Test case 1
    std::string result1 = registerUser("user1", "password123");
    std::cout << "User1 registration: " << result1 << std::endl;
    
    // Test case 2
    std::string result2 = registerUser("alice", "securePass!@#");
    std::cout << "Alice registration: " << result2 << std::endl;
    
    // Test case 3
    std::string result3 = registerUser("bob", "mySecret");
    std::cout << "Bob registration: " << result3 << std::endl;
    
    // Test case 4
    std::string result4 = registerUser("admin", "Admin@2024");
    std::cout << "Admin registration: " << result4 << std::endl;
    
    // Test case 5
    std::string result5 = registerUser("test_user", "Test1234");
    std::cout << "Test_user registration: " << result5 << std::endl;
    
    return 0;
}
