#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

class Task173 {
public:
    /**
     * Finds the longest duplicated substring in a string.
     * @param s The input string.
     * @return The longest duplicated substring, or "" if none exists.
     */
    std::string longestDupSubstring(std::string s) {
        int n = s.length();
        if (n <= 1) {
            return "";
        }

        long long base = 29;
        long long mod = 1e9 + 7;

        // Lambda function to check for duplicated substrings of a given length
        auto check = [&](int len) -> int {
            if (len == 0) return 0;
            if (len >= n) return -1;

            long long h = 1;
            for (int i = 0; i < len; ++i) {
                h = (h * base) % mod;
            }

            std::unordered_set<long long> seen;
            long long current_hash = 0;

            for (int i = 0; i < len; ++i) {
                current_hash = (current_hash * base + (s[i] - 'a' + 1)) % mod;
            }
            seen.insert(current_hash);

            for (int i = 1; i <= n - len; ++i) {
                current_hash = (current_hash * base - ((long long)(s[i - 1] - 'a' + 1) * h) % mod + mod) % mod;
                current_hash = (current_hash + (s[i + len - 1] - 'a' + 1)) % mod;
                
                if (seen.count(current_hash)) {
                    // Found a potential duplicate
                    return i;
                }
                seen.insert(current_hash);
            }
            return -1;
        };
        
        int low = 1, high = n - 1;
        std::string ans = "";
        
        // Binary search for the length
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int start_index = check(mid);
            if (start_index != -1) {
                ans = s.substr(start_index, mid);
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        
        return ans;
    }
};

int main() {
    Task173 solver;
    std::vector<std::string> testCases = {
        "banana",
        "abcd",
        "ababa",
        "aaaaa",
        "mississippi"
    };

    for (const auto& s : testCases) {
        std::string result = solver.longestDupSubstring(s);
        std::cout << "Input: s = \"" << s << "\"" << std::endl;
        std::cout << "Output: \"" << result << "\"" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}