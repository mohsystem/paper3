#include <iostream>
#include <random>
#include <string>
#include <stdexcept>

// Global random engine, seeded once for better performance and randomness
std::mt19937& get_random_engine() {
    static std::random_device rd;
    static std::mt19937 engine(rd());
    return engine;
}

/**
 * @brief Generates a random integer within a specified range (inclusive).
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return A random integer between min and max.
 */
int generateRandomNumber(int min, int max) {
    if (min > max) {
        throw std::invalid_argument("min must not be greater than max");
    }
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(get_random_engine());
}

/**
 * @brief Generates a random alphanumeric token of a specified length.
 * @param length The desired length of the token.
 * @return A random alphanumeric token as a std::string.
 */
std::string generateRandomToken(int length) {
    if (length <= 0) {
        return "";
    }
    const std::string tokenChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string token;
    token.reserve(length);
    std::uniform_int_distribution<int> distribution(0, tokenChars.length() - 1);
    
    for (int i = 0; i < length; ++i) {
        token += tokenChars[distribution(get_random_engine())];
    }
    return token;
}

int main() {
    std::cout << "--- C++ Random Number and Token Generation ---" << std::endl;

    // Test cases for random number generation
    std::cout << "\n--- Random Numbers ---" << std::endl;
    std::cout << "1. Random number between 1 and 100: " << generateRandomNumber(1, 100) << std::endl;
    std::cout << "2. Random number between -50 and 50: " << generateRandomNumber(-50, 50) << std::endl;
    std::cout << "3. Random number between 1000 and 2000: " << generateRandomNumber(1000, 2000) << std::endl;
    std::cout << "4. Random number between 0 and 1: " << generateRandomNumber(0, 1) << std::endl;
    std::cout << "5. Random number between 5 and 5: " << generateRandomNumber(5, 5) << std::endl;

    // Test cases for random token generation
    std::cout << "\n--- Random Tokens ---" << std::endl;
    std::cout << "1. Token of length 8: " << generateRandomToken(8) << std::endl;
    std::cout << "2. Token of length 12: " << generateRandomToken(12) << std::endl;
    std::cout << "3. Token of length 16: " << generateRandomToken(16) << std::endl;
    std::cout << "4. Token of length 1: " << generateRandomToken(1) << std::endl;
    std::cout << "5. Token of length 32: " << generateRandomToken(32) << std::endl;

    return 0;
}