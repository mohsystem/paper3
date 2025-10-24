#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Tweaks letters in a string based on a vector of integer shifts.
 * 
 * @param str The input string.
 * @param tweaks A vector of integers representing the shift for each character.
 * @return The tweaked string, or an error message if inputs are invalid.
 */
std::string tweakLetters(const std::string& str, const std::vector<int>& tweaks) {
    if (str.length() != tweaks.size()) {
        // In C++, throwing an exception (e.g., std::invalid_argument) 
        // is the standard way to handle such errors.
        return "Invalid input";
    }

    std::string result = "";
    result.reserve(str.length()); // Pre-allocate memory for efficiency

    for (size_t i = 0; i < str.length(); ++i) {
        result += static_cast<char>(str[i] + tweaks[i]);
    }
    return result;
}

int main() {
    // Test Case 1
    std::cout << tweakLetters("apple", {0, 1, -1, 0, -1}) << std::endl;
    // Test Case 2
    std::cout << tweakLetters("many", {0, 0, 0, -1}) << std::endl;
    // Test Case 3
    std::cout << tweakLetters("rhino", {1, 1, 1, 1, 1}) << std::endl;
    // Test Case 4
    std::cout << tweakLetters("abc", {-1, 0, 1}) << std::endl;
    // Test Case 5
    std::cout << tweakLetters("z", {1}) << std::endl;
    
    return 0;
}