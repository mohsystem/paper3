#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Task174 {
private:
    /**
     * Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
     * @param pattern The string for which to compute the LPS array.
     * @return The LPS array as a std::vector<int>.
     */
    std::vector<int> computeLPS(const std::string& pattern) {
        int n = pattern.length();
        std::vector<int> lps(n, 0);
        int length = 0; // Length of the previous longest prefix suffix
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
     * Finds the shortest palindrome by adding characters in front of the string.
     * This implementation uses the KMP algorithm's preprocessing step (LPS array)
     * to find the longest palindromic prefix of the string efficiently.
     *
     * @param s The input string.
     * @return The shortest palindrome string.
     */
    std::string shortestPalindrome(std::string s) {
        if (s.length() <= 1) {
            return s;
        }

        std::string rev_s = s;
        std::reverse(rev_s.begin(), rev_s.end());

        // Construct a temporary string: s + # + reverse(s)
        // The '#' is a separator to avoid matching across the two parts.
        std::string temp = s + "#" + rev_s;
        
        // Compute the LPS array
        std::vector<int> lps = computeLPS(temp);

        // The last value in the LPS array gives the length of the longest
        // palindromic prefix of the original string 's'.
        int lps_len = lps.back();

        // The part of the string that is not part of the palindromic prefix
        // needs to be reversed and prepended.
        std::string suffix_to_add = s.substr(lps_len);
        std::string prefix_to_add = suffix_to_add;
        std::reverse(prefix_to_add.begin(), prefix_to_add.end());

        return prefix_to_add + s;
    }
};

int main() {
    Task174 solution;

    std::string test_cases[] = {"aacecaaa", "abcd", "a", "", "abacaba"};
    std::string expected_results[] = {"aaacecaaa", "dcbabcd", "a", "", "abacaba"};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        std::string s = test_cases[i];
        std::string expected = expected_results[i];
        std::string result = solution.shortestPalindrome(s);
        std::cout << "Input: \"" << s << "\"" << std::endl;
        std::cout << "Output: \"" << result << "\"" << std::endl;
        std::cout << "Expected: \"" << expected << "\"" << std::endl;
        std::cout << (result == expected ? "Test PASSED" : "Test FAILED") << std::endl;
        std::cout << "--------------------" << std::endl;
    }
    
    return 0;
}