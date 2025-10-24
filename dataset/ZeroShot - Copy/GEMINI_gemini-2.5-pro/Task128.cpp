#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <stdexcept>

/**
 * Generates a cryptographically secure random integer within a specified range.
 * std::random_device is often implemented using a hardware entropy source,
 * making it suitable for cryptographic purposes on most modern platforms.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @return A secure random integer.
 * @throws std::invalid_argument if max is less than min.
 */
int generateSecureRandomNumber(int min, int max) {
    if (min > max) {
        throw std::invalid_argument("Max must be greater than or equal to Min.");
    }
    // Use std::random_device as a source of non-deterministic random numbers
    std::random_device rd;
    // Define the range of the distribution
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rd);
}

/**
 * Generates a cryptographically secure random alphanumeric token of a given length.
 *
 * @param length The desired length of the token.
 * @return A secure random token string.
 * @throws std::invalid_argument if length is not positive.
 */
std::string generateSecureToken(size_t length) {
    if (length == 0) {
        throw std::invalid_argument("Token length must be positive.");
    }

    static const char token_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    
    std::string token;
    token.reserve(length);
    
    std::random_device rd;
    // The range for the distribution is [0, last_valid_index]
    std::uniform_int_distribution<int> dist(0, sizeof(token_chars) - 2);

    for (size_t i = 0; i < length; ++i) {
        token += token_chars[dist(rd)];
    }

    return token;
}

int main() {
    std::cout << "--- Secure Random Number Generation ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        int min = i * 10;
        int max = 50 + i * 10;
        try {
            int randomNumber = generateSecureRandomNumber(min, max);
            std::cout << "Test " << i + 1 << ": Random number between " << min << " and " << max << ": " << randomNumber << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error generating number: " << e.what() << std::endl;
        }
    }

    std::cout << "\n--- Secure Random Token Generation ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        size_t length = 8 + i * 4;
        try {
            std::string token = generateSecureToken(length);
            std::cout << "Test " << i + 1 << ": Token of length " << length << ": " << token << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error generating token: " << e.what() << std::endl;
        }
    }

    return 0;
}