
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>

class Task56 {
private:
    static const int TOKEN_LENGTH = 32;
    
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        
        std::string ret;
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
                    ret += base64_chars[char_array_4[i]];
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
                ret += base64_chars[char_array_4[j]];
        }
        
        return ret;
    }
    
public:
    static std::string generateToken(const std::string& userId, long long timestamp = -1) {
        if (timestamp == -1) {
            timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }
        
        // Generate secure random bytes
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        unsigned char randomBytes[TOKEN_LENGTH];
        for (int i = 0; i < TOKEN_LENGTH; ++i) {
            randomBytes[i] = static_cast<unsigned char>(dis(gen));
        }
        
        // Combine data
        std::string randomBytesB64 = base64_encode(randomBytes, TOKEN_LENGTH);
        std::string combinedData = userId + ":" + std::to_string(timestamp) + ":" + randomBytesB64;
        
        // Hash using SHA-256
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(combinedData.c_str()), 
               combinedData.length(), hash);
        
        // Encode to Base64
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
};

int main() {
    // Test case 1: Generate token for user1
    std::string token1 = Task56::generateToken("user1");
    std::cout << "Test 1 - Token for user1: " << token1 << std::endl;
    
    // Test case 2: Generate token for user2
    std::string token2 = Task56::generateToken("user2");
    std::cout << "Test 2 - Token for user2: " << token2 << std::endl;
    
    // Test case 3: Generate token with specific timestamp
    long long specificTimestamp = 1700000000000LL;
    std::string token3 = Task56::generateToken("user3", specificTimestamp);
    std::cout << "Test 3 - Token for user3 with timestamp: " << token3 << std::endl;
    
    // Test case 4: Verify tokens are unique for same user
    std::string token4a = Task56::generateToken("user4");
    std::string token4b = Task56::generateToken("user4");
    std::cout << "Test 4 - Two tokens for user4 are different: " 
              << (token4a != token4b ? "true" : "false") << std::endl;
    
    // Test case 5: Generate token for admin user
    std::string token5 = Task56::generateToken("admin");
    std::cout << "Test 5 - Token for admin: " << token5 << std::endl;
    
    return 0;
}
