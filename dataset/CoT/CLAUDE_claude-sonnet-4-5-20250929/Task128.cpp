
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fstream>
#endif

class Task128 {
private:
    static const std::string ALPHANUMERIC;
    
    // Generate cryptographically secure random bytes
    static void generateSecureBytes(unsigned char* buffer, size_t length) {
#ifdef _WIN32
        // Windows: Use BCryptGenRandom
        if (BCryptGenRandom(NULL, buffer, static_cast<ULONG>(length), 
                           BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
            throw std::runtime_error("Failed to generate secure random bytes");
        }
#else
        // Unix/Linux: Use /dev/urandom
        std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
        if (!urandom) {
            throw std::runtime_error("Failed to open /dev/urandom");
        }
        urandom.read(reinterpret_cast<char*>(buffer), length);
        if (!urandom) {
            throw std::runtime_error("Failed to read from /dev/urandom");
        }
        urandom.close();
#endif
    }
    
public:
    /**
     * Generates a cryptographically secure random integer within a range
     */
    static int generateSecureRandomInt(int min, int max) {
        if (min > max) {
            throw std::invalid_argument("min must be less than or equal to max");
        }
        
        unsigned char randomBytes[4];
        generateSecureBytes(randomBytes, 4);
        
        unsigned int randomValue = 0;
        for (int i = 0; i < 4; i++) {
            randomValue = (randomValue << 8) | randomBytes[i];
        }
        
        int range = max - min + 1;
        return (randomValue % range) + min;
    }
    
    /**
     * Generates a cryptographically secure hexadecimal token
     */
    static std::string generateHexToken(size_t byteLength) {
        if (byteLength == 0) {
            throw std::invalid_argument("byteLength must be positive");
        }
        
        unsigned char* randomBytes = new unsigned char[byteLength];
        generateSecureBytes(randomBytes, byteLength);
        
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < byteLength; i++) {
            oss << std::setw(2) << static_cast<int>(randomBytes[i]);
        }
        
        delete[] randomBytes;
        return oss.str();
    }
    
    /**
     * Generates a cryptographically secure alphanumeric token
     */
    static std::string generateAlphanumericToken(size_t length) {
        if (length == 0) {
            throw std::invalid_argument("length must be positive");
        }
        
        std::string token;
        token.reserve(length);
        
        for (size_t i = 0; i < length; i++) {
            unsigned char randomByte;
            generateSecureBytes(&randomByte, 1);
            token += ALPHANUMERIC[randomByte % ALPHANUMERIC.length()];
        }
        
        return token;
    }
    
    /**
     * Generates a cryptographically secure random long
     */
    static long long generateSecureRandomLong() {
        unsigned char randomBytes[8];
        generateSecureBytes(randomBytes, 8);
        
        long long result = 0;
        for (int i = 0; i < 8; i++) {
            result = (result << 8) | randomBytes[i];
        }
        
        return result;
    }
};

const std::string Task128::ALPHANUMERIC = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int main() {
    std::cout << "=== Cryptographically Secure Random Number/Token Generator ===" << std::endl << std::endl;
    
    try {
        // Test Case 1: Random integer in range
        std::cout << "Test Case 1: Random integer between 1 and 100" << std::endl;
        std::cout << "Result: " << Task128::generateSecureRandomInt(1, 100) << std::endl << std::endl;
        
        // Test Case 2: Hexadecimal token (32 bytes)
        std::cout << "Test Case 2: Hexadecimal token (32 bytes)" << std::endl;
        std::cout << "Result: " << Task128::generateHexToken(32) << std::endl << std::endl;
        
        // Test Case 3: Alphanumeric token (16 characters)
        std::cout << "Test Case 3: Alphanumeric token (16 chars)" << std::endl;
        std::cout << "Result: " << Task128::generateAlphanumericToken(16) << std::endl << std::endl;
        
        // Test Case 4: Hexadecimal token (16 bytes)
        std::cout << "Test Case 4: Hexadecimal token (16 bytes)" << std::endl;
        std::cout << "Result: " << Task128::generateHexToken(16) << std::endl << std::endl;
        
        // Test Case 5: Random long value
        std::cout << "Test Case 5: Random long value" << std::endl;
        std::cout << "Result: " << Task128::generateSecureRandomLong() << std::endl << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
