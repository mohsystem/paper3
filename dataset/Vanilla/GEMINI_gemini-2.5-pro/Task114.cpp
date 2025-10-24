#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::transform
#include <cctype>    // For toupper and tolower

/**
 * Creates a copy of a string.
 * @param s The source string.
 * @return A new string with the same content as s.
 */
std::string copyString(const std::string& s) {
    return std::string(s);
}

/**
 * Concatenates two strings.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return The combined string.
 */
std::string concatenateStrings(const std::string& s1, const std::string& s2) {
    return s1 + s2;
}

/**
 * Gets the length of a string.
 * @param s The input string.
 * @return The length of the string.
 */
size_t getStringLength(const std::string& s) {
    return s.length();
}

/**
 * Finds the index of a substring within a main string.
 * @param mainStr The string to search in.
 * @param subStr The substring to search for.
 * @return The starting index of the substring, or std::string::npos if not found.
 */
size_t findSubstring(const std::string& mainStr, const std::string& subStr) {
    return mainStr.find(subStr);
}

/**
 * Converts a string to uppercase.
 * @param s The input string.
 * @return The uppercase version of the string.
 */
std::string toUpperCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}

/**
 * Converts a string to lowercase.
 * @param s The input string.
 * @return The lowercase version of the string.
 */
std::string toLowerCase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

int main() {
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"Hello", "World"},
        {"Cpp", "Programming"},
        {"Test", "Case"},
        {"one two three", "two"},
        {"UPPER", "lower"}
    };

    for (int i = 0; i < testCases.size(); ++i) {
        std::string s1 = testCases[i].first;
        std::string s2 = testCases[i].second;

        std::cout << "--- Test Case " << i + 1 << " ---" << std::endl;
        std::cout << "Original strings: \"" << s1 << "\", \"" << s2 << "\"" << std::endl;

        // Copy
        std::string copied = copyString(s1);
        std::cout << "Copied s1: " << copied << std::endl;

        // Concatenate
        std::string concatenated = concatenateStrings(s1, s2);
        std::cout << "Concatenated: " << concatenated << std::endl;

        // Length
        std::cout << "Length of s1: " << getStringLength(s1) << std::endl;

        // Find Substring
        size_t index;
        if (s1 == "one two three" && s2 == "two") { // special case for test 4
            index = findSubstring(s1, s2);
             std::cout << "Index of \"" << s2 << "\" in \"" << s1 << "\": ";
        } else {
            index = findSubstring(s1, "o");
            std::cout << "Index of 'o' in \"" << s1 << "\": ";
        }
        if (index == std::string::npos) {
            std::cout << "not found" << std::endl;
        } else {
            std::cout << index << std::endl;
        }

        // To Uppercase
        std::cout << "s1 to uppercase: " << toUpperCase(s1) << std::endl;

        // To Lowercase
        std::cout << "s2 to lowercase: " << toLowerCase(s2) << std::endl;
        std::cout << std::endl;
    }

    return 0;
}