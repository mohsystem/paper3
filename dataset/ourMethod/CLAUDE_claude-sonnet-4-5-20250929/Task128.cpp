
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <openssl/rand.h>
#include <openssl/err.h>

// Secure random number generator using OpenSSL
// CWE-338 mitigation: Use cryptographically secure RNG, never std::rand
class SecureRandomGenerator {
public:
    // Generate a secure random integer in range [min, max]
    // CWE-190 mitigation: Validate range and check for integer overflow
    static int64_t generateSecureRandomInt(int64_t min, int64_t max) {
        if (min > max) {
            throw std::invalid_argument("min must be <= max");
        }
        
        // Check for potential overflow in range calculation
        if (max > 0 && min < 0 && (max - min) < 0) {
            throw std::overflow_error("Range too large");
        }
        
        uint64_t range = static_cast<uint64_t>(max - min);
        
        // Use rejection sampling to avoid modulo bias
        // CWE-327 mitigation: Use cryptographically strong random bytes from OpenSSL
        uint64_t random_value;
        const uint64_t rejection_threshold = (UINT64_MAX - range) % (range + 1);
        
        do {
            // CWE-330 mitigation: Use cryptographically secure random source
            if (RAND_bytes(reinterpret_cast<unsigned char*>(&random_value), 
                          sizeof(random_value)) != 1) {
                throw std::runtime_error("Failed to generate secure random bytes");
            }
        } while (random_value < rejection_threshold);
        
        return min + static_cast<int64_t>(random_value % (range + 1));
    }
    
    // Generate a secure random token (hex string)
    // CWE-330 mitigation: Use CSPRNG for token generation
    static std::string generateSecureToken(size_t byte_length) {
        // CWE-129 mitigation: Validate input length to prevent excessive allocation
        if (byte_length == 0 || byte_length > 1024) {
            throw std::invalid_argument("byte_length must be between 1 and 1024");
        }
        
        // CWE-401 mitigation: Use RAII with std::vector for automatic memory management
        std::vector<unsigned char> buffer(byte_length);
        
        // CWE-327 mitigation: Generate cryptographically secure random bytes
        if (RAND_bytes(buffer.data(), static_cast<int>(byte_length)) != 1) {
            throw std::runtime_error("Failed to generate secure random token");
        }
        
        // Convert to hex string
        std::ostringstream oss;
        for (size_t i = 0; i < byte_length; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0') 
                << static_cast<int>(buffer[i]);
        }
        
        return oss.str();
    }
    
    // Generate secure random bytes
    // CWE-330 mitigation: Provide raw secure random bytes for cryptographic operations
    static std::vector<uint8_t> generateSecureBytes(size_t length) {
        // CWE-129 mitigation: Validate input length
        if (length == 0 || length > 1024) {
            throw std::invalid_argument("length must be between 1 and 1024");
        }
        
        std::vector<uint8_t> buffer(length);
        
        // CWE-327 mitigation: Use OpenSSL CSPRNG
        if (RAND_bytes(buffer.data(), static_cast<int>(length)) != 1) {
            throw std::runtime_error("Failed to generate secure random bytes");
        }
        
        return buffer;
    }
};

int main() {
    try {
        std::cout << "=== Secure Random Number and Token Generator ===" << std::endl;
        std::cout << std::endl;
        
        // Test Case 1: Generate random integers in range [1, 100]
        std::cout << "Test 1: Generate 5 random integers in range [1, 100]" << std::endl;
        for (int i = 0; i < 5; ++i) {
            int64_t random_num = SecureRandomGenerator::generateSecureRandomInt(1, 100);
            std::cout << "  Random number " << (i + 1) << ": " << random_num << std::endl;
        }
        std::cout << std::endl;
        
        // Test Case 2: Generate random integers in range [0, 255]
        std::cout << "Test 2: Generate 5 random integers in range [0, 255]" << std::endl;
        for (int i = 0; i < 5; ++i) {
            int64_t random_num = SecureRandomGenerator::generateSecureRandomInt(0, 255);
            std::cout << "  Random number " << (i + 1) << ": " << random_num << std::endl;
        }
        std::cout << std::endl;
        
        // Test Case 3: Generate 16-byte (128-bit) secure tokens
        std::cout << "Test 3: Generate 5 secure tokens (16 bytes each)" << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::string token = SecureRandomGenerator::generateSecureToken(16);
            std::cout << "  Token " << (i + 1) << ": " << token << std::endl;
        }
        std::cout << std::endl;
        
        // Test Case 4: Generate 32-byte (256-bit) secure tokens
        std::cout << "Test 4: Generate 5 secure tokens (32 bytes each)" << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::string token = SecureRandomGenerator::generateSecureToken(32);
            std::cout << "  Token " << (i + 1) << ": " << token << std::endl;
        }
        std::cout << std::endl;
        
        // Test Case 5: Generate secure random bytes
        std::cout << "Test 5: Generate 5 sets of secure random bytes (8 bytes each)" << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::vector<uint8_t> bytes = SecureRandomGenerator::generateSecureBytes(8);
            std::cout << "  Bytes " << (i + 1) << ": ";
            for (size_t j = 0; j < bytes.size(); ++j) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                         << static_cast<int>(bytes[j]);
            }
            std::cout << std::dec << std::endl;
        }
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        // CWE-209 mitigation: Don't expose sensitive error details
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
