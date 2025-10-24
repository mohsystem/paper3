#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <utility>

/**
 * Returns the number of distinct non-empty substrings of text that can be written
 * as the concatenation of some string with itself.
 *
 * @param text The input string.
 * @return The number of distinct echo substrings.
 */
int distinctEchoSubstrings(const std::string& text) {
    int n = text.length();
    if (n == 0) {
        return 0;
    }

    // Use double hashing to minimize collisions.
    long long p1 = 31, m1 = 1e9 + 7;
    long long p2 = 37, m2 = 1e9 + 9;

    std::vector<long long> p_pow1(n + 1);
    std::vector<long long> h1(n + 1, 0);
    std::vector<long long> p_pow2(n + 1);
    std::vector<long long> h2(n + 1, 0);

    p_pow1[0] = 1;
    p_pow2[0] = 1;

    for (int i = 0; i < n; ++i) {
        p_pow1[i + 1] = (p_pow1[i] * p1) % m1;
        h1[i + 1] = (h1[i] * p1 + (text[i] - 'a' + 1)) % m1;
        p_pow2[i + 1] = (p_pow2[i] * p2) % m2;
        h2[i + 1] = (h2[i] * p2 + (text[i] - 'a' + 1)) % m2;
    }
    
    // Custom hash for std::pair to be used in std::unordered_set
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator () (const std::pair<T1, T2>& p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            // A common way to combine hashes
            return h1 ^ (h2 << 1);
        }
    };

    std::unordered_set<std::pair<long long, long long>, pair_hash> distinct_hashes;

    // len is the length of the full substring 'a+a'
    for (int len = 2; len <= n; len += 2) {
        int half = len / 2;
        for (int i = 0; i <= n - len; ++i) {
            int j = i + half;
            int k = i + len;

            // Hash of first half: text[i...j-1]
            long long hash1_a = (h1[j] - (h1[i] * p_pow1[half]) % m1 + m1) % m1;
            // Hash of second half: text[j...k-1]
            long long hash1_b = (h1[k] - (h1[j] * p_pow1[half]) % m1 + m1) % m1;

            if (hash1_a == hash1_b) {
                long long hash2_a = (h2[j] - (h2[i] * p_pow2[half]) % m2 + m2) % m2;
                long long hash2_b = (h2[k] - (h2[j] * p_pow2[half]) % m2 + m2) % m2;

                if (hash2_a == hash2_b) {
                    // Hashes of halves match, confirming an echo substring.
                    // Add the hash of the full substring text[i...k-1] to the set.
                    long long full_hash1 = (h1[k] - (h1[i] * p_pow1[len]) % m1 + m1) % m1;
                    long long full_hash2 = (h2[k] - (h2[i] * p_pow2[len]) % m2 + m2) % m2;
                    distinct_hashes.insert({full_hash1, full_hash2});
                }
            }
        }
    }

    return distinct_hashes.size();
}

void runTestCase(const std::string& text, int expected) {
    int result = distinctEchoSubstrings(text);
    std::cout << "Input: \"" << text << "\", Output: " << result << std::endl;
}

int main() {
    runTestCase("abcabcabc", 3);
    runTestCase("leetcodeleetcode", 2);
    runTestCase("abacaba", 0);
    runTestCase("aaaaa", 2);
    runTestCase("aaaaaaaaaa", 4);
    return 0;
}