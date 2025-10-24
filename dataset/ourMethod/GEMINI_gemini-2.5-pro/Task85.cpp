#include <iostream>
#include <string>
#include <random>
#include <vector>

/**
 * @brief Generates a cryptographically secure random string of a given length.
 * 
 * The string will consist only of ASCII letters (a-z, A-Z).
 * NOTE: The security of this function depends on the quality of std::random_device,
 * which is implementation-defined. On most modern platforms (Linux, Windows, macOS),
 * it is a cryptographically secure random number generator.
 *
 * @param length The desired length of the string.
 * @return A random string of the specified length, or an empty string if length is <= 0.
 */
std::string generateRandomString(int length) {
    if (length <= 0) {
        return "";
    }

    static const char ALPHABET[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int alphabetSize = sizeof(ALPHABET) - 1;

    std::string result;
    result.reserve(length);
    
    try {
        std::random_device rd;
        std::uniform_int_distribution<int> distribution(0, alphabetSize - 1);

        for (int i = 0; i < length; ++i) {
            result += ALPHABET[distribution(rd)];
        }
    } catch (const std::exception& e) {
        // std::random_device might throw an exception if a source of randomness is not available
        std::cerr << "Error generating random string: " << e.what() << std::endl;
        return "";
    }

    return result;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Length 10
    std::string s1 = generateRandomString(10);
    std::cout << "Test 1 (length 10): " << s1 << " (length: " << s1.length() << ")" << std::endl;

    // Test Case 2: Length 0
    std::string s2 = generateRandomString(0);
    std::cout << "Test 2 (length 0): \"" << s2 << "\" (length: " << s2.length() << ")" << std::endl;

    // Test Case 3: Length 1
    std::string s3 = generateRandomString(1);
    std::cout << "Test 3 (length 1): " << s3 << " (length: " << s3.length() << ")" << std::endl;

    // Test Case 4: Length 32
    std::string s4 = generateRandomString(32);
    std::cout << "Test 4 (length 32): " << s4 << " (length: " << s4.length() << ")" << std::endl;

    // Test Case 5: Negative length
    std::string s5 = generateRandomString(-5);
    std::cout << "Test 5 (length -5): \"" << s5 << "\" (length: " << s5.length() << ")" << std::endl;

    return 0;
}