
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>

// Simple Base64 encoding
std::string base64_encode(const unsigned char* data, size_t length) {
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    int i = 0;
    unsigned char array3[3];
    unsigned char array4[4];

    while (length--) {
        array3[i++] = *(data++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            for(i = 0; i < 4; i++) result += base64_chars[array4[i]];
            i = 0;
        }
    }

    if (i) {
        for(int j = i; j < 3; j++) array3[j] = '\\0';
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        for (int j = 0; j < i + 1; j++) result += base64_chars[array4[j]];
    }

    return result;
}

// Simple SHA-256 implementation (simplified for demonstration)
void sha256_simple(const std::string& input, unsigned char output[32]) {
    // Note: In production, use a proper crypto library like OpenSSL
    // This is a placeholder that creates a pseudo-hash
    std::hash<std::string> hasher;
    size_t hash_value = hasher(input);
    
    for (int i = 0; i < 32; i++) {
        output[i] = (hash_value >> (i % 8)) ^ (input[i % input.length()] * (i + 1));
    }
}

std::string generateToken(const std::string& userId) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    unsigned char randomBytes[32];
    for (int i = 0; i < 32; i++) {
        randomBytes[i] = dis(gen);
    }
    
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    std::string randomBase64 = base64_encode(randomBytes, 32);
    std::string dataToHash = userId + std::to_string(timestamp) + randomBase64;
    
    unsigned char hash[32];
    sha256_simple(dataToHash, hash);
    
    return base64_encode(hash, 32);
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    std::string token1 = generateToken("user123");
    std::cout << "Token for user123: " << token1 << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::string token2 = generateToken("admin@example.com");
    std::cout << "Token for admin@example.com: " << token2 << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::string token3 = generateToken("testUser456");
    std::cout << "Token for testUser456: " << token3 << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::string token4 = generateToken("john.doe");
    std::cout << "Token for john.doe: " << token4 << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::string token5 = generateToken("user123");
    std::cout << "Token for user123 (second generation): " << token5 << std::endl;
    std::cout << "Tokens are unique: " << (token1 != token5 ? "true" : "false") << std::endl;
    
    return 0;
}
