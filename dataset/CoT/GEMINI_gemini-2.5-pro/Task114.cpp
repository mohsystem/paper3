#include <iostream>
#include <string>
#include <vector>

/**
 * Creates a copy of the given string.
 * @param source The string to copy.
 * @return A new string with the same content as the source.
 */
std::string copyString(const std::string& source) {
    return source; // std::string handles copying automatically
}

/**
 * Concatenates two strings.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A new string that is the result of s1 + s2.
 */
std::string concatenateStrings(const std::string& s1, const std::string& s2) {
    return s1 + s2;
}

/**
 * Gets the length of a string.
 * @param s The input string.
 * @return The length of the string as a size_t.
 */
size_t getStringLength(const std::string& s) {
    return s.length();
}

/**
 * Finds the first occurrence of a substring within a main string.
 * @param mainStr The string to search within.
 * @param subStr The substring to search for.
 * @return The starting index of the substring, or std::string::npos if not found.
 */
size_t findSubstring(const std::string& mainStr, const std::string& subStr) {
    return mainStr.find(subStr);
}

/**
 * Replaces all occurrences of a substring with a new substring.
 * Note: This creates a new string with the replacements.
 * @param mainStr The original string.
 * @param oldSub The substring to be replaced.
 * @param newSub The substring to replace with.
 * @return A new string with all replacements made.
 */
std::string replaceSubstring(std::string mainStr, const std::string& oldSub, const std::string& newSub) {
    if (oldSub.empty()) {
        return mainStr;
    }
    size_t start_pos = 0;
    while ((start_pos = mainStr.find(oldSub, start_pos)) != std::string::npos) {
        mainStr.replace(start_pos, oldSub.length(), newSub);
        start_pos += newSub.length(); // Move past the new substring
    }
    return mainStr;
}

int main() {
    // Test Case 1: Copying a string
    std::string original1 = "Hello World";
    std::string copied1 = copyString(original1);
    std::cout << "Test Case 1: Copying a String" << std::endl;
    std::cout << "Original:  \"" << original1 << "\"" << std::endl;
    std::cout << "Copied:    \"" << copied1 << "\"" << std::endl << std::endl;

    // Test Case 2: Concatenating strings
    std::string s1 = "Hello, ";
    std::string s2 = "World!";
    std::string concatenated = concatenateStrings(s1, s2);
    std::cout << "Test Case 2: Concatenating Strings" << std::endl;
    std::cout << "String 1: \"" << s1 << "\"" << std::endl;
    std::cout << "String 2: \"" << s2 << "\"" << std::endl;
    std::cout << "Concatenated: \"" << concatenated << "\"" << std::endl << std::endl;

    // Test Case 3: Getting string length
    std::string s3 = "Programming";
    size_t length = getStringLength(s3);
    std::cout << "Test Case 3: Getting String Length" << std::endl;
    std::cout << "String: \"" << s3 << "\"" << std::endl;
    std::cout << "Length: " << length << std::endl << std::endl;

    // Test Case 4: Finding a substring
    std::string mainStr4 = "This is a test";
    std::string subStr4 = "is";
    size_t index = findSubstring(mainStr4, subStr4);
    std::cout << "Test Case 4: Finding a Substring" << std::endl;
    std::cout << "Main String: \"" << mainStr4 << "\"" << std::endl;
    std::cout << "Substring:   \"" << subStr4 << "\"" << std::endl;
    if (index != std::string::npos) {
        std::cout << "Found at index: " << index << std::endl << std::endl;
    } else {
        std::cout << "Substring not found." << std::endl << std::endl;
    }

    // Test Case 5: Replacing a substring
    std::string mainStr5 = "The house is blue and the car is blue.";
    std::string oldSub5 = "blue";
    std::string newSub5 = "red";
    std::string replaced = replaceSubstring(mainStr5, oldSub5, newSub5);
    std::cout << "Test Case 5: Replacing a Substring" << std::endl;
    std::cout << "Original: \"" << mainStr5 << "\"" << std::endl;
    std::cout << "Replaced: \"" << replaced << "\"" << std::endl << std::endl;

    return 0;
}