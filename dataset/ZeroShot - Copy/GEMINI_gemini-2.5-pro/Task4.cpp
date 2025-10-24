#include <iostream>
#include <string>
#include <vector>
#include <cctype>   // For std::tolower
#include <array>    // For std::array

/**
 * @brief Determines if a string is an isogram.
 * An isogram is a word that has no repeating letters. Case is ignored.
 * Assumes the input string contains only letters.
 * 
 * @param str The string to check.
 * @return true if the string is an isogram, false otherwise.
 */
bool isIsogram(const std::string& str) {
    // A boolean array to mark the presence of characters 'a' through 'z'.
    std::array<bool, 26> seen{}; // Value-initializes all elements to false

    for (char c : str) {
        // Convert character to lowercase
        char lower_c = std::tolower(static_cast<unsigned char>(c));
        int index = lower_c - 'a';

        // If we have seen this character before, it's not an isogram.
        if (seen[index]) {
            return false;
        }
        // Mark this character as seen.
        seen[index] = true;
    }

    return true;
}

int main() {
    std::vector<std::string> testCases = {
        "Dermatoglyphics",
        "aba",
        "moOse",
        "",
        "isogram"
    };

    // Use std::boolalpha to print boolean values as "true" or "false"
    std::cout << std::boolalpha;

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& test : testCases) {
        std::cout << "\"" << test << "\" --> " << isIsogram(test) << std::endl;
    }

    return 0;
}