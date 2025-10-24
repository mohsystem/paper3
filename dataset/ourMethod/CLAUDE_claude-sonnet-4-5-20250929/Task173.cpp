
#include <string>
#include <unordered_set>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

// Use Rabin-Karp rolling hash with binary search to find longest duplicate substring
// Security: All inputs validated, no dynamic code execution, bounds checked
class Solution {
private:
    static const uint64_t BASE = 26;
    static const uint64_t MOD = 1000000007; // Large prime to reduce collisions
    
    // Check if there exists a duplicate substring of given length
    // Returns the duplicate substring if found, empty string otherwise
    std::string hasDuplicate(const std::string& s, int len) {
        // Input validation: check length bounds
        if (len <= 0 || len > static_cast<int>(s.length())) {
            return "";
        }
        
        // Precompute BASE^(len-1) % MOD for rolling hash
        uint64_t basePower = 1;
        for (int i = 0; i < len - 1; ++i) {
            basePower = (basePower * BASE) % MOD;
        }
        
        // Compute initial hash for first window
        uint64_t hash = 0;
        for (int i = 0; i < len; ++i) {
            // Validate character is lowercase letter
            if (s[i] < 'a' || s[i] > 'z') {
                return ""; // Invalid input
            }
            hash = (hash * BASE + static_cast<uint64_t>(s[i] - 'a')) % MOD;
        }
        
        // Store seen hashes with their starting positions
        std::unordered_set<uint64_t> seen;
        seen.insert(hash);
        
        // Rolling hash: slide window through string
        // Bounds check: i + len <= s.length() ensures no overflow
        for (size_t i = 1; i + len <= s.length(); ++i) {
            // Validate characters
            if (s[i - 1] < 'a' || s[i - 1] > 'z' || 
                s[i + len - 1] < 'a' || s[i + len - 1] > 'z') {
                return ""; // Invalid input
            }
            
            // Remove leading character and add trailing character
            hash = (hash + MOD - (basePower * static_cast<uint64_t>(s[i - 1] - 'a')) % MOD) % MOD;
            hash = (hash * BASE + static_cast<uint64_t>(s[i + len - 1] - 'a')) % MOD;
            
            if (seen.count(hash)) {
                // Hash collision check: verify actual substring match
                std::string candidate = s.substr(i, len);
                // Search for actual duplicate in previous positions
                for (size_t j = 0; j < i; ++j) {
                    if (j + len <= s.length() && s.substr(j, len) == candidate) {
                        return candidate;
                    }
                }
            }
            seen.insert(hash);
        }
        
        return "";
    }
    
public:
    std::string longestDupSubstring(const std::string& s) {
        // Input validation: check constraints
        if (s.length() < 2 || s.length() > 30000) {
            return "";
        }
        
        // Validate all characters are lowercase letters
        for (char c : s) {
            if (c < 'a' || c > 'z') {
                return "";
            }
        }
        
        // Binary search on length of duplicate substring
        int left = 1;
        int right = static_cast<int>(s.length()) - 1;
        std::string result = "";
        
        while (left <= right) {
            // Overflow protection: use safe midpoint calculation
            int mid = left + (right - left) / 2;
            std::string dup = hasDuplicate(s, mid);
            
            if (!dup.empty()) {
                result = dup;
                left = mid + 1; // Try longer substrings
            } else {
                right = mid - 1; // Try shorter substrings
            }
        }
        
        return result;
    }
};

int main() {
    Solution sol;
    
    // Test case 1
    std::string test1 = "banana";
    std::string result1 = sol.longestDupSubstring(test1);
    std::cout << "Test 1: " << result1 << " (expected: ana)" << std::endl;
    assert(result1 == "ana");
    
    // Test case 2
    std::string test2 = "abcd";
    std::string result2 = sol.longestDupSubstring(test2);
    std::cout << "Test 2: " << result2 << " (expected: empty)" << std::endl;
    assert(result2 == "");
    
    // Test case 3
    std::string test3 = "aa";
    std::string result3 = sol.longestDupSubstring(test3);
    std::cout << "Test 3: " << result3 << " (expected: a)" << std::endl;
    assert(result3 == "a");
    
    // Test case 4
    std::string test4 = "aaaaa";
    std::string result4 = sol.longestDupSubstring(test4);
    std::cout << "Test 4: " << result4 << " (expected: aaaa)" << std::endl;
    assert(result4 == "aaaa");
    
    // Test case 5
    std::string test5 = "abcabcabc";
    std::string result5 = sol.longestDupSubstring(test5);
    std::cout << "Test 5: " << result5 << " (expected: abcabc)" << std::endl;
    assert(result5 == "abcabc");
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
