#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Task174 {
private:
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

public:
    std::string shortestPalindrome(std::string s) {
        if (s.length() <= 1) {
            return s;
        }

        int n = s.length();
        std::string rev_s = s;
        std::reverse(rev_s.begin(), rev_s.end());
        
        std::string temp = s + '#' + rev_s;
        
        std::vector<int> lps = computeLPSArray(temp);
        int l = lps.back();
        
        std::string prefix = rev_s.substr(0, n - l);
        
        return prefix + s;
    }
};

int main() {
    Task174 solution;

    // Test Case 1
    std::string s1 = "aacecaaa";
    std::cout << "Input: " << s1 << std::endl;
    std::cout << "Output: " << solution.shortestPalindrome(s1) << std::endl;
    std::cout << std::endl;

    // Test Case 2
    std::string s2 = "abcd";
    std::cout << "Input: " << s2 << std::endl;
    std::cout << "Output: " << solution.shortestPalindrome(s2) << std::endl;
    std::cout << std::endl;

    // Test Case 3
    std::string s3 = "ababa";
    std::cout << "Input: " << s3 << std::endl;
    std::cout << "Output: " << solution.shortestPalindrome(s3) << std::endl;
    std::cout << std::endl;

    // Test Case 4
    std::string s4 = "race";
    std::cout << "Input: " << s4 << std::endl;
    std::cout << "Output: " << solution.shortestPalindrome(s4) << std::endl;
    std::cout << std::endl;

    // Test Case 5
    std::string s5 = "a";
    std::cout << "Input: " << s5 << std::endl;
    std::cout << "Output: " << solution.shortestPalindrome(s5) << std::endl;
    std::cout << std::endl;

    return 0;
}