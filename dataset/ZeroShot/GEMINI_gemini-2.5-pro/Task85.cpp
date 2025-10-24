#include <iostream>
#include <string>
#include <random>
#include <vector>

/**
 * @brief Generates a cryptographically secure random string of a specified length.
 * The string consists only of ASCII letters (a-z, A-Z).
 * This implementation uses std::random_device, which is a source of non-deterministic 
 * random numbers if a hardware entropy source is available.
 * 
 * @param length The desired length of the string. Must be non-negative.
 * @return A random string of the specified length, or an empty string if length is <= 0.
 */
std::string generateRandomString(int length) {
    if (length <= 0) {
        return "";
    }

    const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string random_string;
    random_string.reserve(length);

    std::random_device rd;
    std::uniform_int_distribution<> distrib(0, CHARACTERS.length() - 1);

    for (int i = 0; i < length; ++i) {
        random_string += CHARACTERS[distrib(rd)];
    }

    return random_string;
}

int main() {
    std::cout << "CPP Test Cases:" << std::endl;
    int testLengths[] = {16, 32, 1, 0, -5};
    for (int len : testLengths) {
        std::string randomStr = generateRandomString(len);
        std::cout << "Length " << len << " -> \"" << randomStr << "\" (Actual length: " << randomStr.length() << ")" << std::endl;
    }
    return 0;
}