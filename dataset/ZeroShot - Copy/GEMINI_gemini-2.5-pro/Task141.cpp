#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // for std::reverse

/**
 * @brief Reverses a given string.
 * 
 * This function is secure as it uses std::string which handles its own memory,
 * preventing buffer overflows. It operates on a copy of the input string
 * and does not modify the original.
 * 
 * @param str The string to be reversed (passed as a const reference for efficiency).
 * @return A new string containing the reversed content.
 */
std::string reverseString(const std::string& str) {
    std::string reversed_str = str;
    std::reverse(reversed_str.begin(), reversed_str.end());
    return reversed_str;
}

int main() {
    std::vector<std::string> testCases = {
        "hello",
        "CPP",
        "12345",
        "",
        "a single char"
    };

    std::cout << "--- C++ String Reversal ---" << std::endl;
    for (const auto& test : testCases) {
        std::string reversed = reverseString(test);
        std::cout << "Original: \"" << test << "\", Reversed: \"" << reversed << "\"" << std::endl;
    }

    return 0;
}