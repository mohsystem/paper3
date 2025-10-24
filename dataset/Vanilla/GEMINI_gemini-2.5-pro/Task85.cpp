#include <iostream>
#include <string>
#include <random>
#include <stdexcept>

/**
 * Generates an unpredictable random string of a given length.
 * The string consists only of ASCII letters (uppercase and lowercase).
 * @param length The length of the string to generate.
 * @return The randomly generated string.
 */
std::string generateRandomString(int length) {
    if (length < 0) {
        throw std::invalid_argument("Length must be a non-negative number.");
    }
    if (length == 0) {
        return "";
    }

    static const std::string CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    // Use a static random engine to avoid re-seeding on every call
    static std::random_device rd;
    static std::mt19937 generator(rd());
    
    std::uniform_int_distribution<int> distribution(0, CHARACTERS.length() - 1);

    std::string random_string;
    random_string.reserve(length);

    for (int i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

int main() {
    std::cout << "CPP Test Cases:" << std::endl;

    int testLengths[] = {10, 15, 8, 0, 25};

    for (int len : testLengths) {
        try {
            std::string randomString = generateRandomString(len);
            std::cout << "Length " << len << ": " << randomString << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error for length " << len << ": " << e.what() << std::endl;
        }
    }

    return 0;
}