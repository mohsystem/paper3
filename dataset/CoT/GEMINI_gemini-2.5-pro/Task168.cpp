#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Tweaks letters in a string based on a vector of integers.
 * 
 * @param str The input string (all lowercase letters).
 * @param arr A vector of integers (0, 1, or -1) of the same length as str.
 * @return std::string The modified string.
 */
std::string tweakLetters(const std::string& str, const std::vector<int>& arr) {
    if (str.length() != arr.size()) {
        // As per prompt, we assume valid inputs.
        // In a real-world scenario, you might throw an exception.
        return "";
    }

    std::string result = str; // Create a mutable copy
    for (size_t i = 0; i < str.length(); ++i) {
        result[i] = static_cast<char>(str[i] + arr[i]);
    }
    return result;
}

int main() {
    // Test Case 1
    std::cout << "tweakLetters(\"apple\", {0, 1, -1, 0, -1}) -> \""
              << tweakLetters("apple", {0, 1, -1, 0, -1}) << "\"" << std::endl;

    // Test Case 2
    std::cout << "tweakLetters(\"many\", {0, 0, 0, -1}) -> \""
              << tweakLetters("many", {0, 0, 0, -1}) << "\"" << std::endl;

    // Test Case 3
    std::cout << "tweakLetters(\"rhino\", {1, 1, 1, 1, 1}) -> \""
              << tweakLetters("rhino", {1, 1, 1, 1, 1}) << "\"" << std::endl;

    // Test Case 4
    std::cout << "tweakLetters(\"abc\", {-1, 0, 1}) -> \""
              << tweakLetters("abc", {-1, 0, 1}) << "\"" << std::endl;

    // Test Case 5
    std::cout << "tweakLetters(\"zulu\", {1, -1, 1, -1}) -> \""
              << tweakLetters("zulu", {1, -1, 1, -1}) << "\"" << std::endl;

    return 0;
}