
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fstream>
#endif

// CWE-330, CWE-338: Use cryptographically secure random number generator
// Never use std::rand() for security-sensitive operations
class SecureRandom {
public:
    // Generate cryptographically secure random bytes
    static void getBytes(unsigned char* buffer, size_t length) {
        if (buffer == nullptr || length == 0) {
            throw std::invalid_argument("Invalid buffer or length");
        }

#if defined(_WIN32)
        // Windows: Use BCryptGenRandom for CSPRNG
        NTSTATUS status = BCryptGenRandom(
            nullptr,
            buffer,
            static_cast<ULONG>(length),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG
        );
        if (status != 0) {
            throw std::runtime_error("Failed to generate secure random bytes");
        }
#else
        // Linux/Unix: Read from /dev/urandom (cryptographically secure)
        std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
        if (!urandom) {
            throw std::runtime_error("Failed to open /dev/urandom");
        }
        urandom.read(reinterpret_cast<char*>(buffer), length);
        if (!urandom || urandom.gcount() != static_cast<std::streamsize>(length)) {
            throw std::runtime_error("Failed to read sufficient random bytes");
        }
#endif
    }
};

// CWE-129: Validate input length to prevent integer overflow and resource exhaustion
// CWE-703: Handle errors gracefully with proper validation
std::string generateRandomString(int length) {
    // Input validation: length must be positive and reasonable
    // Prevent integer overflow and excessive memory allocation
    if (length < 0) {
        throw std::invalid_argument("Length must be non-negative");
    }
    if (length == 0) {
        return "";
    }
    // Limit maximum length to prevent resource exhaustion (CWE-770)
    const int MAX_LENGTH = 1000000; // 1MB limit for safety
    if (length > MAX_LENGTH) {
        throw std::invalid_argument("Length exceeds maximum allowed size");
    }

    // ASCII letters: A-Z (65-90) and a-z (97-122) = 52 total letters
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t numLetters = 52;

    // Allocate buffer for random bytes
    // Use vector for RAII and automatic memory management
    std::vector<unsigned char> randomBytes(length);
    
    try {
        // Generate cryptographically secure random bytes
        SecureRandom::getBytes(randomBytes.data(), length);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Random generation failed: ") + e.what());
    }

    // Build result string with proper bounds checking
    std::string result;
    result.reserve(length); // Pre-allocate to avoid reallocations

    for (int i = 0; i < length; ++i) {
        // Use modulo to map random byte to valid letter index
        // This provides uniform distribution over ASCII letters
        size_t index = randomBytes[i] % numLetters;
        result += letters[index];
    }

    // Clear sensitive random data from memory (defense in depth)
    // Use volatile to prevent compiler optimization
    volatile unsigned char* ptr = randomBytes.data();
    for (size_t i = 0; i < randomBytes.size(); ++i) {
        ptr[i] = 0;
    }

    return result;
}

int main() {
    try {
        // Test case 1: Small string
        std::cout << "Test 1 (length 8): " << generateRandomString(8) << std::endl;

        // Test case 2: Medium string
        std::cout << "Test 2 (length 16): " << generateRandomString(16) << std::endl;

        // Test case 3: Larger string
        std::cout << "Test 3 (length 32): " << generateRandomString(32) << std::endl;

        // Test case 4: Empty string
        std::cout << "Test 4 (length 0): \\"" << generateRandomString(0) << "\\"" << std::endl;

        // Test case 5: Single character
        std::cout << "Test 5 (length 1): " << generateRandomString(1) << std::endl;

        // Demonstrate error handling for invalid input
        try {
            generateRandomString(-5);
        } catch (const std::exception& e) {
            std::cout << "Expected error for negative length: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
