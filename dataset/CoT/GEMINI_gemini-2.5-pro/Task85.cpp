#include <iostream>
#include <string>
#include <random>
#include <vector>

/**
 * @brief Generates an unpredictable random string of a specified length using only ASCII letters.
 *
 * This function uses std::random_device to seed a Mersenne Twister engine. 
 * On compliant implementations, std::random_device produces non-deterministic random numbers,
 * making the generated string unpredictable.
 *
 * @param length The desired length of the string.
 * @return A random string of the specified length, or an empty string if length is <= 0.
 */
std::string generateRandomString(int length) {
    if (length <= 0) {
        return "";
    }

    const std::string CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(length);

    // Use std::random_device as a source of non-deterministic random numbers
    std::random_device rd;
    // Seed the Mersenne Twister engine
    std::mt19937 generator(rd());
    // Create a uniform distribution to map random numbers to indices in our character set
    std::uniform_int_distribution<> distribution(0, CHARS.length() - 1);

    for (int i = 0; i < length; ++i) {
        result += CHARS[distribution(generator)];
    }

    return result;
}

int main() {
    std::cout << "\nCPP Test Cases:" << std::endl;

    // Test Case 1: Standard length
    int len1 = 16;
    std::string randomStr1 = generateRandomString(len1);
    std::cout << "1. Length " << len1 << ": " << randomStr1 << " (Actual length: " << randomStr1.length() << ")" << std::endl;

    // Test Case 2: Short length
    int len2 = 5;
    std::string randomStr2 = generateRandomString(len2);
    std::cout << "2. Length " << len2 << ": " << randomStr2 << " (Actual length: " << randomStr2.length() << ")" << std::endl;

    // Test Case 3: Long length
    int len3 = 64;
    std::string randomStr3 = generateRandomString(len3);
    std::cout << "3. Length " << len3 << ": " << randomStr3 << " (Actual length: " << randomStr3.length() << ")" << std::endl;

    // Test Case 4: Zero length
    int len4 = 0;
    std::string randomStr4 = generateRandomString(len4);
    std::cout << "4. Length " << len4 << ": \"" << randomStr4 << "\" (Actual length: " << randomStr4.length() << ")" << std::endl;

    // Test Case 5: Negative length
    int len5 = -10;
    std::string randomStr5 = generateRandomString(len5);
    std::cout << "5. Length " << len5 << ": \"" << randomStr5 << "\" (Actual length: " << randomStr5.length() << ")" << std::endl;

    return 0;
}