#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Task174 {
private:
    /**
     * Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
     * @param pattern The string for which to compute the LPS array.
     * @return A vector representing the LPS array.
     */
    std::vector<int> computeLPSArray(const std::string& pattern) {
        int n = pattern.length();
        std::vector<int> lps(n, 0);
        int length = 0; // length of the previous longest prefix suffix
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

public:
    /**
     * Finds the shortest palindrome by adding characters to the front of the string.
     * @param s The input string.
     * @return The shortest palindrome.
     */
    std::string shortestPalindrome(std::string s) {
        if (s.length() <= 1) {
            return s;
        }

        std::string reversed_s = s;
        std::reverse(reversed_s.begin(), reversed_s.end());

        std::string temp = s + "#" + reversed_s;
        
        std::vector<int> lps = computeLPSArray(temp);
        int palindromicPrefixLength = lps.back();

        std::string suffixToPrepend = s.substr(palindromicPrefixLength);
        std::reverse(suffixToPrepend.begin(), suffixToPrepend.end());

        return suffixToPrepend + s;
    }
};

int main() {
    Task174 solution;

    // Test Case 1
    std::string s1 = "aacecaaa";
    std::cout << "Input: \"" << s1 << "\", Output: \"" << solution.shortestPalindrome(s1) << "\"" << std::endl;

    // Test Case 2
    std::string s2 = "abcd";
    std::cout << "Input: \"" << s2 << "\", Output: \"" << solution.shortestPalindrome(s2) << "\"" << std::endl;

    // Test Case 3
    std::string s3 = "abacaba";
    std::cout << "Input: \"" << s3 << "\", Output: \"" << solution.shortestPalindrome(s3) << "\"" << std::endl;

    // Test Case 4
    std::string s4 = "";
    std::cout << "Input: \"" << s4 << "\", Output: \"" << solution.shortestPalindrome(s4) << "\"" << std::endl;

    // Test Case 5
    std::string s5 = "abab";
    std::cout << "Input: \"" << s5 << "\", Output: \"" << solution.shortestPalindrome(s5) << "\"" << std::endl;

    return 0;
}