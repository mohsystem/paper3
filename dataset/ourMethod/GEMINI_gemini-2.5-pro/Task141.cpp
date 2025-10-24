#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::reverse

/**
 * Reverses a given string.
 *
 * @param str The input string (passed by const reference for efficiency).
 * @return A new string that is the reverse of the input.
 */
std::string reverseString(const std::string& str) {
    // Create a copy of the input string to be reversed.
    std::string reversed_str = str;
    // Use std::reverse from the <algorithm> header to reverse the string in-place.
    std::reverse(reversed_str.begin(), reversed_str.end());
    return reversed_str;
}

int main() {
    std::vector<std::string> testCases = {
        "Hello, World!",
        "madam",
        "12345",
        "",
        "a"
    };

    for (const auto& testCase : testCases) {
        std::cout << "Original: \"" << testCase << "\"" << std::endl;
        std::string reversed = reverseString(testCase);
        std::cout << "Reversed: \"" << reversed << "\"" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}