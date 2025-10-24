#include <iostream>
#include <string>
#include <algorithm> // Required for std::reverse
#include <vector>

/**
 * Reverses a given string.
 *
 * @param str The string to be reversed. Passed by const reference to avoid copying.
 * @return A new string containing the reversed content.
 */
std::string reverseString(const std::string& str) {
    // Create a new string from the reverse iterators of the input string.
    // This is an efficient and idiomatic C++ way to reverse a string.
    return std::string(str.rbegin(), str.rend());
}

int main() {
    // 5 test cases
    std::vector<std::string> testCases = {
        "hello world",
        "12345",
        "a",
        "",
        "Palindrome"
    };

    for (const auto& test : testCases) {
        std::string reversed = reverseString(test);
        std::cout << "Original: \"" << test << "\", Reversed: \"" << reversed << "\"" << std::endl;
    }

    return 0;
}