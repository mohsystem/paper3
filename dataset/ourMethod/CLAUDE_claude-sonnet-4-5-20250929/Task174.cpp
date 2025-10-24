
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

// Security: Input validation - ensure string length is within constraints
// and contains only lowercase English letters
bool isValidInput(const std::string& s) {
    // Check length constraint: 0 <= s.length <= 5 * 10^4
    if (s.length() > 50000) {
        return false;
    }
    
    // Validate that string contains only lowercase English letters
    for (size_t i = 0; i < s.length(); ++i) {
        if (s[i] < 'a' || s[i] > 'z') {
            return false;
        }
    }
    return true;
}

// KMP algorithm to find longest prefix which is also suffix
// Security: Using std::vector for safe memory management
std::vector<int> computeLPSArray(const std::string& pattern) {
    size_t n = pattern.length();
    std::vector<int> lps(n, 0);
    
    // Security: Bounds checking - ensure we don't exceed vector size
    if (n == 0) {
        return lps;
    }
    
    int len = 0;
    size_t i = 1;
    
    // Security: Loop bounds are checked against vector size
    while (i < n) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    
    return lps;
}

std::string shortestPalindrome(const std::string& s) {
    // Security: Input validation
    if (!isValidInput(s)) {
        return ""; // Return empty string for invalid input
    }
    
    size_t n = s.length();
    
    // Edge case: empty string or single character is already a palindrome
    if (n <= 1) {
        return s;
    }
    
    // Create reverse of string
    std::string rev = s;
    std::reverse(rev.begin(), rev.end());
    
    // Create combined string: s + "#" + reverse(s)
    // Security: Using std::string operations which handle memory safely
    std::string combined = s + "#" + rev;
    
    // Security: Check for potential overflow before creating combined string
    if (combined.length() < s.length()) {
        return ""; // Overflow detected
    }
    
    // Compute LPS array for combined string
    std::vector<int> lps = computeLPSArray(combined);
    
    // The last value in LPS tells us the longest prefix of s that matches suffix of rev
    // This is the longest palindrome starting from beginning of s
    int palindromeLen = lps[combined.length() - 1];
    
    // Characters to add = total length - palindrome length
    // Security: Bounds checking before substring operation
    if (palindromeLen < 0 || static_cast<size_t>(palindromeLen) > n) {
        return ""; // Invalid state
    }
    
    size_t charsToAdd = n - palindromeLen;
    
    // Security: Validate substring bounds
    if (charsToAdd > n) {
        return ""; // Invalid calculation
    }
    
    // Add reverse of non-palindrome suffix to the front
    std::string prefix = rev.substr(0, charsToAdd);
    
    return prefix + s;
}

int main() {
    // Test case 1
    std::string test1 = "aacecaaa";
    std::cout << "Input: \\"" << test1 << "\\"" << std::endl;
    std::cout << "Output: \\"" << shortestPalindrome(test1) << "\\"" << std::endl;
    std::cout << std::endl;
    
    // Test case 2
    std::string test2 = "abcd";
    std::cout << "Input: \\"" << test2 << "\\"" << std::endl;
    std::cout << "Output: \\"" << shortestPalindrome(test2) << "\\"" << std::endl;
    std::cout << std::endl;
    
    // Test case 3: empty string
    std::string test3 = "";
    std::cout << "Input: \\"" << test3 << "\\"" << std::endl;
    std::cout << "Output: \\"" << shortestPalindrome(test3) << "\\"" << std::endl;
    std::cout << std::endl;
    
    // Test case 4: single character
    std::string test4 = "a";
    std::cout << "Input: \\"" << test4 << "\\"" << std::endl;
    std::cout << "Output: \\"" << shortestPalindrome(test4) << "\\"" << std::endl;
    std::cout << std::endl;
    
    // Test case 5: already a palindrome
    std::string test5 = "aba";
    std::cout << "Input: \\"" << test5 << "\\"" << std::endl;
    std::cout << "Output: \\"" << shortestPalindrome(test5) << "\\"" << std::endl;
    std::cout << std::endl;
    
    return 0;
}
