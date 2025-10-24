#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

/**
 * Reverses a given string.
 * @param str The string to be reversed.
 * @return The reversed string.
 */
std::string reverseString(const std::string& str) {
    return std::string(str.rbegin(), str.rend());
}

int main() {
    std::vector<std::string> testCases = {
        "hello",
        "world",
        "CPP",
        "12345",
        ""
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& test : testCases) {
        std::string reversed = reverseString(test);
        std::cout << "Original: \"" << test << "\", Reversed: \"" << reversed << "\"" << std::endl;
    }

    return 0;
}