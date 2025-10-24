#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

// The main class is named Task173 as per instructions
class Task173 {
private:
    std::string s_val;
    std::vector<long long> power;
    long long base = 26;
    long long modulus = 2147483647; // 2^31 - 1, a prime number

    // Helper function to check for duplicates of a given length using rolling hash
    int search(int len) {
        int n = s_val.length();
        if (len == 0) return 0;
        if (len >= n) return -1;

        long long currentHash = 0;
        for (int i = 0; i < len; ++i) {
            currentHash = (currentHash * base + (s_val[i] - 'a')) % modulus;
        }

        std::unordered_set<long long> seen;
        seen.insert(currentHash);
        
        long long highPower = power[len - 1]; // base^(len-1)

        for (int i = 1; i <= n - len; ++i) {
            // Rolling hash update
            currentHash = (currentHash - (long long)(s_val[i - 1] - 'a') * highPower) % modulus;
            currentHash = (currentHash * base + (s_val[i + len - 1] - 'a')) % modulus;
            
            // Handle negative results from modulo
            if (currentHash < 0) {
                currentHash += modulus;
            }

            if (seen.count(currentHash)) {
                // Hash match found. Assuming it's a true positive.
                return i; 
            }
            seen.insert(currentHash);
        }
        return -1;
    }

public:
    std::string longestDupSubstring(std::string s) {
        this->s_val = s;
        int n = s_val.length();
        if (n <= 1) return "";

        // Precompute powers of the base
        power.resize(n);
        power[0] = 1;
        for (int i = 1; i < n; ++i) {
            power[i] = (power[i - 1] * base) % modulus;
        }

        int low = 1, high = n - 1;
        int start = -1;
        int maxLen = 0;

        // Binary search for the length
        while (low <= high) {
            int mid = low + (high - low) / 2;
            int foundStart = search(mid);
            if (foundStart != -1) {
                maxLen = mid;
                start = foundStart;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        if (start == -1) {
            return "";
        }
        return s_val.substr(start, maxLen);
    }
};

void run_test(Task173& solver, const std::string& s, const std::string& expected) {
    std::string result = solver.longestDupSubstring(s);
    std::cout << "Input: s = \"" << s << "\"" << std::endl;
    std::cout << "Output: \"" << result << "\"" << std::endl;
    std::cout << "Expected: \"" << expected << "\"" << std::endl;
    std::cout << std::endl;
}

int main() {
    Task173 solver;

    std::vector<std::pair<std::string, std::string>> testCases = {
        {"banana", "ana"},
        {"abcd", ""},
        {"ababa", "aba"},
        {"zzzzzzzzzz", "zzzzzzzzz"},
        {"mississippi", "issi"}
    };

    int i = 1;
    for (const auto& test : testCases) {
        std::cout << "Test Case " << i++ << ":" << std::endl;
        run_test(solver, test.first, test.second);
    }

    return 0;
}