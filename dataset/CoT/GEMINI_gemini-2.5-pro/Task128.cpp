#include <iostream>
#include <string>
#include <random>
#include <stdexcept>
#include <vector>

// Note: std::random_device is intended to be a source of non-deterministic
// randomness, but its quality is implementation-defined. For high-security
// applications, platform-specific APIs or dedicated crypto libraries (like OpenSSL)
// are recommended over the standard C++ library.

/**
 * Generates a high-quality random integer within a specified range.
 * @param min The minimum value (inclusive).
 * @param max The maximum value (inclusive).
 * @return A random integer between min and max.
 */
int generateRandomInt(int min, int max) {
    if (min >= max) {
        throw std::invalid_argument("max must be greater than min");
    }
    // Use a static random engine to avoid re-seeding on every call
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

/**
 * Generates a high-quality random alphanumeric token of a specified length.
 * @param length The desired length of the token.
 * @return A random alphanumeric token.
 */
std::string generateRandomToken(int length) {
    if (length <= 0) {
        throw std::invalid_argument("Token length must be positive.");
    }
    static const std::string CHAR_SET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, CHAR_SET.length() - 1);

    std::string token;
    token.reserve(length);
    for (int i = 0; i < length; ++i) {
        token += CHAR_SET[distrib(gen)];
    }
    return token;
}

int main() {
    try {
        std::cout << "C++ Random Generation Test Cases:" << std::endl;

        // Test Case 1
        std::cout << "1. Random integer between 1 and 100: " << generateRandomInt(1, 100) << std::endl;

        // Test Case 2
        std::cout << "2. Random integer between -50 and 50: " << generateRandomInt(-50, 50) << std::endl;

        // Test Case 3
        std::cout << "3. Random token of length 8: " << generateRandomToken(8) << std::endl;

        // Test Case 4
        std::cout << "4. Random token of length 16: " << generateRandomToken(16) << std::endl;

        // Test Case 5
        std::cout << "5. Random integer between 1000 and 5000: " << generateRandomInt(1000, 5000) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}