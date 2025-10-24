#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Tweaks letters in a string by one forward (+1) or backward (-1) according to a vector.
 *
 * @param str The input string (assumed to be lowercase letters).
 * @param tweaks A vector of integers (0, 1, or -1) of the same length as str.
 * @return A new string with the tweaked letters.
 * @throw std::invalid_argument if inputs have mismatched lengths.
 */
std::string tweakLetters(const std::string& str, const std::vector<int>& tweaks) {
    if (str.length() != tweaks.size()) {
        throw std::invalid_argument("Input string and tweaks vector must have the same length.");
    }

    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i) {
        result += static_cast<char>(str[i] + tweaks[i]);
    }

    return result;
}

int main() {
    // Test Case 1
    std::cout << "Test 1: 'apple', {0, 1, -1, 0, -1} -> " << tweakLetters("apple", {0, 1, -1, 0, -1}) << std::endl;
    
    // Test Case 2
    std::cout << "Test 2: 'many', {0, 0, 0, -1} -> " << tweakLetters("many", {0, 0, 0, -1}) << std::endl;
    
    // Test Case 3
    std::cout << "Test 3: 'rhino', {1, 1, 1, 1, 1} -> " << tweakLetters("rhino", {1, 1, 1, 1, 1}) << std::endl;
    
    // Test Case 4: Empty string
    std::cout << "Test 4: '', {} -> " << tweakLetters("", {}) << std::endl;
    
    // Test Case 5: All zeros
    std::cout << "Test 5: 'hello', {0, 0, 0, 0, 0} -> " << tweakLetters("hello", {0, 0, 0, 0, 0}) << std::endl;

    return 0;
}