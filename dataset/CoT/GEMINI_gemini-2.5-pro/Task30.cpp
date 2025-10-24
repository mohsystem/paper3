#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

/**
 * @brief Takes 2 strings s1 and s2 including only letters from a to z.
 * Returns a new sorted string, the longest possible, containing distinct letters.
 * 
 * @param s1 The first string.
 * @param s2 The second string.
 * @return The longest sorted string with distinct letters from s1 and s2.
 */
std::string longest(const std::string& s1, const std::string& s2) {
    bool seen[26] = {false}; // for 'a' through 'z'

    // Mark characters from s1 as seen
    for (char c : s1) {
        if (c >= 'a' && c <= 'z') {
            seen[c - 'a'] = true;
        }
    }

    // Mark characters from s2 as seen
    for (char c : s2) {
        if (c >= 'a' && c <= 'z') {
            seen[c - 'a'] = true;
        }
    }

    // Build the result string from the seen characters
    std::string result = "";
    for (int i = 0; i < 26; ++i) {
        if (seen[i]) {
            result += (char)('a' + i);
        }
    }

    return result;
}

int main() {
    // Test Case 1
    std::string a1 = "xyaabbbccccdefww";
    std::string b1 = "xxxxyyyyabklmopq";
    std::cout << "Test Case 1:" << std::endl;
    std::cout << "s1: \"" << a1 << "\"" << std::endl;
    std::cout << "s2: \"" << b1 << "\"" << std::endl;
    std::cout << "Result: \"" << longest(a1, b1) << "\"" << std::endl; // Expected: "abcdefklmopqwxy"
    std::cout << std::endl;

    // Test Case 2
    std::string a2 = "abcdefghijklmnopqrstuvwxyz";
    std::string b2 = "abcdefghijklmnopqrstuvwxyz";
    std::cout << "Test Case 2:" << std::endl;
    std::cout << "s1: \"" << a2 << "\"" << std::endl;
    std::cout << "s2: \"" << b2 << "\"" << std::endl;
    std::cout << "Result: \"" << longest(a2, b2) << "\"" << std::endl; // Expected: "abcdefghijklmnopqrstuvwxyz"
    std::cout << std::endl;

    // Test Case 3
    std::string a3 = "aretheyhere";
    std::string b3 = "yestheyarehere";
    std::cout << "Test Case 3:" << std::endl;
    std::cout << "s1: \"" << a3 << "\"" << std::endl;
    std::cout << "s2: \"" << b3 << "\"" << std::endl;
    std::cout << "Result: \"" << longest(a3, b3) << "\"" << std::endl; // Expected: "aehrsty"
    std::cout << std::endl;

    // Test Case 4
    std::string a4 = "loopingisfunbutdangerous";
    std::string b4 = "lessdangerousthancoding";
    std::cout << "Test Case 4:" << std::endl;
    std::cout << "s1: \"" << a4 << "\"" << std::endl;
    std::cout << "s2: \"" << b4 << "\"" << std::endl;
    std::cout << "Result: \"" << longest(a4, b4) << "\"" << std::endl; // Expected: "abcdefghilnoprstu"
    std::cout << std::endl;

    // Test Case 5
    std::string a5 = "";
    std::string b5 = "";
    std::cout << "Test Case 5:" << std::endl;
    std::cout << "s1: \"" << a5 << "\"" << std::endl;
    std::cout << "s2: \"" << b5 << "\"" << std::endl;
    std::cout << "Result: \"" << longest(a5, b5) << "\"" << std::endl; // Expected: ""
    std::cout << std::endl;

    return 0;
}