#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Tweaks letters by one forward (+1) or backwards (-1) according to a vector.
 * 
 * @param str The input string of lowercase letters.
 * @param tweaks A vector of integers (0, 1, or -1).
 * @return The tweaked string.
 */
std::string tweakLetters(const std::string& str, const std::vector<int>& tweaks) {
    if (str.length() != tweaks.size()) {
        return ""; // Or throw an exception
    }
    std::string result = "";
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        int tweak = tweaks[i];
        char newChar = c + tweak;

        if (newChar > 'z') {
            newChar = 'a';
        } else if (newChar < 'a') {
            newChar = 'z';
        }
        result += newChar;
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
    std::cout << tweakLetters("zebra", {1, -1, 1, -1, 1}) << std::endl;

    // Test Case 5
    std::cout << tweakLetters("abc", {-1, 0, 1}) << std::endl;
    
    return 0;
}