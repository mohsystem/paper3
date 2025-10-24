#include <iostream>
#include <string>
#include <random>
#include <vector>

/**
 * Generates a random alphanumeric token of a specified length.
 * @param length The desired length of the token.
 * @return A std::string containing the generated token.
 */
std::string generateToken(int length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string token;
    token.reserve(length);

    // Use a random_device to seed the pseudo-random number generator.
    // This is a better source of randomness than time-based seeds.
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.length() - 1);

    for (int i = 0; i < length; ++i) {
        token += charset[distribution(generator)];
    }

    return token;
}

int main() {
    std::cout << "CPP: Generating 5 authentication tokens." << std::endl;
    for (int i = 0; i < 5; ++i) {
        // Generate a 32-character token
        std::string token = generateToken(32);
        std::cout << "Test Case " << (i + 1) << ": " << token << std::endl;
    }
    return 0;
}