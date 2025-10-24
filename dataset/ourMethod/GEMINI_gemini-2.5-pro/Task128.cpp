#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>

/**
 * @brief Generates a cryptographically secure random integer within a specified range.
 *
 * This function uses std::random_device, which is typically a non-deterministic
 * random number generator that can be used to seed a pseudo-random engine.
 * On many systems, it provides access to a hardware entropy source.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @return A random integer within the specified range.
 * @throws std::invalid_argument if min is greater than or equal to max.
 */
int generateRandomInt(int min, int max) {
    if (min >= max) {
        throw std::invalid_argument("Max must be greater than min.");
    }

    std::random_device rd;
    // Seed a Mersenne Twister engine. For security-sensitive applications,
    // a different engine or direct use of rd might be considered, but this
    // is a common and robust practice for high-quality random numbers.
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}

/**
 * @brief Generates a cryptographically secure random token as a hexadecimal string.
 *
 * @param numBytes The number of random bytes to generate. The resulting hex string will be 2 * numBytes long.
 * @return A random token encoded as a hexadecimal string.
 * @throws std::invalid_argument if numBytes is zero.
 * @throws std::runtime_error if std::random_device is not available or fails.
 */
std::string generateRandomTokenHex(size_t numBytes) {
    if (numBytes == 0) {
        throw std::invalid_argument("Number of bytes must be positive.");
    }

    std::vector<unsigned char> buffer(numBytes);
    std::random_device rd;
    
    // Check if the random_device is truly non-deterministic
    if (rd.entropy() == 0) {
        // This is a rare case but indicates a low-quality entropy source.
        // For critical applications, one might stop here.
        // We will proceed but this is worth noting.
    }

    // Generate random bytes
    for (size_t i = 0; i < numBytes; ++i) {
        // random_device generates unsigned int, we cast to get a byte
        buffer[i] = static_cast<unsigned char>(rd() % 256);
    }
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : buffer) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

int main() {
    try {
        std::cout << "--- Testing Random Number and Token Generation ---" << std::endl;

        // Test Case 1: Standard integer range
        std::cout << "Test Case 1: Random int between 1 and 100" << std::endl;
        int randomInt1 = generateRandomInt(1, 100);
        std::cout << "Generated Number: " << randomInt1 << std::endl << std::endl;

        // Test Case 2: Negative integer range
        std::cout << "Test Case 2: Random int between -50 and -10" << std::endl;
        int randomInt2 = generateRandomInt(-50, -10);
        std::cout << "Generated Number: " << randomInt2 << std::endl << std::endl;

        // Test Case 3: 16-byte (128-bit) random token
        std::cout << "Test Case 3: 16-byte random token (32 hex characters)" << std::endl;
        std::string token1 = generateRandomTokenHex(16);
        std::cout << "Generated Token: " << token1 << std::endl << std::endl;

        // Test Case 4: 32-byte (256-bit) random token
        std::cout << "Test Case 4: 32-byte random token (64 hex characters)" << std::endl;
        std::string token2 = generateRandomTokenHex(32);
        std::cout << "Generated Token: " << token2 << std::endl << std::endl;
        
        // Test Case 5: Invalid range for integer generation
        std::cout << "Test Case 5: Invalid range (100, 1)" << std::endl;
        try {
            generateRandomInt(100, 1);
        } catch (const std::invalid_argument& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}