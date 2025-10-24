#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

class Task172 {
public:
    int countDistinct(const std::string& text) {
        int n = text.length();
        if (n <= 1) {
            return 0;
        }

        long long p1 = 31, m1 = 1e9 + 7;
        long long p2 = 37, m2 = 1e9 + 9;

        std::vector<long long> p1_powers(n + 1);
        std::vector<long long> p2_powers(n + 1);
        p1_powers[0] = 1;
        p2_powers[0] = 1;
        for (int i = 1; i <= n; i++) {
            p1_powers[i] = (p1_powers[i - 1] * p1) % m1;
            p2_powers[i] = (p2_powers[i - 1] * p2) % m2;
        }

        std::vector<long long> h1(n + 1, 0);
        std::vector<long long> h2(n + 1, 0);
        for (int i = 0; i < n; i++) {
            h1[i + 1] = (h1[i] * p1 + (text[i] - 'a' + 1)) % m1;
            h2[i + 1] = (h2[i] * p2 + (text[i] - 'a' + 1)) % m2;
        }

        std::unordered_set<long long> foundHashes;

        // len is the length of 'a'
        for (int len = 1; len * 2 <= n; len++) {
            // i is the starting index of 'a+a'
            for (int i = 0; i <= n - 2 * len; i++) {
                int mid_idx = i + len;

                // Hash of the first half
                long long hash1_a = (h1[mid_idx] - (h1[i] * p1_powers[len]) % m1 + m1) % m1;
                long long hash2_a = (h2[mid_idx] - (h2[i] * p2_powers[len]) % m2 + m2) % m2;

                // Hash of the second half
                long long hash1_b = (h1[i + 2 * len] - (h1[mid_idx] * p1_powers[len]) % m1 + m1) % m1;
                long long hash2_b = (h2[i + 2 * len] - (h2[mid_idx] * p2_powers[len]) % m2 + m2) % m2;

                if (hash1_a == hash1_b && hash2_a == hash2_b) {
                    // Hashes match, add the hash of the whole substring 'a+a' to the set
                    long long total_hash1 = (h1[i + 2 * len] - (h1[i] * p1_powers[2 * len]) % m1 + m1) % m1;
                    long long total_hash2 = (h2[i + 2 * len] - (h2[i] * p2_powers[2 * len]) % m2 + m2) % m2;
                    foundHashes.insert(total_hash1 * m2 + total_hash2);
                }
            }
        }
        return foundHashes.size();
    }
};

int main() {
    Task172 solution;

    std::string text1 = "abcabcabc";
    std::cout << "Input: " << text1 << ", Output: " << solution.countDistinct(text1) << std::endl;

    std::string text2 = "leetcodeleetcode";
    std::cout << "Input: " << text2 << ", Output: " << solution.countDistinct(text2) << std::endl;
    
    std::string text3 = "aaaaa";
    std::cout << "Input: " << text3 << ", Output: " << solution.countDistinct(text3) << std::endl;

    std::string text4 = "ababa";
    std::cout << "Input: " << text4 << ", Output: " << solution.countDistinct(text4) << std::endl;

    std::string text5 = "zzzzzzzzzz";
    std::cout << "Input: " << text5 << ", Output: " << solution.countDistinct(text5) << std::endl;
    
    return 0;
}