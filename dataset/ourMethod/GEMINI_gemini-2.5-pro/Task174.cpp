#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

/**
 * @brief Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
 * 
 * @param pattern The string for which to compute the LPS array.
 * @return std::vector<int> The LPS array.
 */
std::vector<int> computeLPSArray(const std::string& pattern) {
    int n = pattern.length();
    std::vector<int> lps(n, 0);
    int length = 0;
    int i = 1;
    while (i < n) {
        if (pattern[i] == pattern[length]) {
            length++;
            lps[i] = length;
            i++;
        } else {
            if (length != 0) {
                length = lps[length - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

/**
 * @brief Finds the shortest palindrome by adding characters to the front of the string.
 * 
 * @param s The input string.
 * @return std::string The shortest palindrome.
 */
std::string shortestPalindrome(const std::string& s) {
    if (s.length() <= 1) {
        return s;
    }

    std::string reversed_s = s;
    std::reverse(reversed_s.begin(), reversed_s.end());

    std::string temp = s + '#' + reversed_s;
    std::vector<int> lps = computeLPSArray(temp);
    int lpsLength = lps.back();

    std::string suffix = s.substr(lpsLength);
    std::reverse(suffix.begin(), suffix.end());

    return suffix + s;
}

int main() {
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"aacecaaa", "aaacecaaa"},
        {"abcd", "dcbabcd"},
        {"aba", "aba"},
        {"", ""},
        {"a", "a"}
    };

    for (int i = 0; i < testCases.size(); ++i) {
        const auto& testCase = testCases[i];
        std::string s = testCase.first;
        std::string expected = testCase.second;
        std::string result = shortestPalindrome(s);
        std::cout << "Input: \"" << s << "\"" << std::endl;
        std::cout << "Output: \"" << result << "\"" << std::endl;
        std::cout << "Expected: \"" << expected << "\"" << std::endl;
        std::cout << "Test " << i + 1 << " " << (result == expected ? "Passed" : "Failed") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}