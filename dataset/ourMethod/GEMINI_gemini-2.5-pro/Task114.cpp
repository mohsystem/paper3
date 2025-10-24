#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // for std::reverse
#include <utility>   // for std::pair

/**
 * @brief Creates a copy of a std::string.
 * std::string's copy constructor or assignment operator handles this safely.
 * @param source The string to copy.
 * @return A new std::string object with the same content.
 */
std::string safeStringCopy(const std::string& source) {
    return source; // Invokes copy constructor, creating a safe copy
}

/**
 * @brief Safely concatenates two std::strings.
 * The overloaded '+' operator for std::string is safe and handles memory management.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A new std::string with the concatenated content.
 */
std::string safeStringConcat(const std::string& s1, const std::string& s2) {
    return s1 + s2;
}

/**
 * @brief Reverses a std::string.
 * @param source The string to reverse.
 * @return A new std::string with the reversed content.
 */
std::string reverseString(const std::string& source) {
    std::string reversed = source;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

int main() {
    std::cout << "--- Testing safeStringCopy ---" << std::endl;
    std::vector<std::string> copyTests = {"hello", "", "a long string example", "another one", "test 5"};
    for (const auto& test : copyTests) {
        std::string copied = safeStringCopy(test);
        std::cout << "Original: \"" << test << "\", Copied: \"" << copied << "\"" << std::endl;
    }

    std::cout << "\n--- Testing safeStringConcat ---" << std::endl;
    std::vector<std::pair<std::string, std::string>> concatTests = {
        {"Hello, ", "World!"},
        {"", "Append this"},
        {"Start with this", ""},
        {"", ""},
        {"Test", " 5"}
    };
    for (const auto& test : concatTests) {
        std::string result = safeStringConcat(test.first, test.second);
        std::cout << "s1: \"" << test.first << "\", s2: \"" << test.second 
                  << "\", Result: \"" << result << "\"" << std::endl;
    }

    std::cout << "\n--- Testing reverseString ---" << std::endl;
    std::vector<std::string> reverseTests = {"abcdef", "12345", "racecar", "a", ""};
    for (const auto& test : reverseTests) {
        std::string reversed = reverseString(test);
        std::cout << "Original: \"" << test << "\", Reversed: \"" << reversed << "\"" << std::endl;
    }

    return 0;
}