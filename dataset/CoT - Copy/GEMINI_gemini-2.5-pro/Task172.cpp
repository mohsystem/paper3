#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <utility>

class Task172 {
public:
    int distinctEchoSubstrings(const std::string& text) {
        int n = text.length();
        long long p1 = 31, m1 = 1e9 + 7;
        long long p2 = 37, m2 = 1e9 + 9;

        std::vector<long long> p1_powers(n + 1);
        std::vector<long long> p2_powers(n + 1);
        std::vector<long long> h1(n + 1, 0);
        std::vector<long long> h2(n + 1, 0);

        p1_powers[0] = 1;
        p2_powers[0] = 1;
        for (int i = 1; i <= n; ++i) {
            p1_powers[i] = (p1_powers[i - 1] * p1) % m1;
            p2_powers[i] = (p2_powers[i - 1] * p2) % m2;
        }

        for (int i = 0; i < n; ++i) {
            h1[i + 1] = (h1[i] * p1 + (text[i] - 'a' + 1)) % m1;
            h2[i + 1] = (h2[i] * p2 + (text[i] - 'a' + 1)) % m2;
        }

        auto get_hashes = [&](int l, int r) {
            int len = r - l + 1;
            long long hash_a = (h1[r + 1] - (h1[l] * p1_powers[len]) % m1 + m1) % m1;
            long long hash_b = (h2[r + 1] - (h2[l] * p2_powers[len]) % m2 + m2) % m2;
            return std::make_pair(hash_a, hash_b);
        };
        
        std::set<std::pair<long long, long long>> found;
        for (int len = 2; len <= n; len += 2) {
            int half = len / 2;
            for (int i = 0; i <= n - len; ++i) {
                if (get_hashes(i, i + half - 1) == get_hashes(i + half, i + len - 1)) {
                    found.insert(get_hashes(i, i + len - 1));
                }
            }
        }
        
        return found.size();
    }
};

int main() {
    Task172 solution;
    
    // Test Case 1
    std::string text1 = "abcabcabc";
    std::cout << "Test Case 1: text = \"" << text1 << "\", Expected: 3, Got: " << solution.distinctEchoSubstrings(text1) << std::endl;

    // Test Case 2
    std::string text2 = "leetcodeleetcode";
    std::cout << "Test Case 2: text = \"" << text2 << "\", Expected: 2, Got: " << solution.distinctEchoSubstrings(text2) << std::endl;

    // Test Case 3
    std::string text3 = "aaaaa";
    std::cout << "Test Case 3: text = \"" << text3 << "\", Expected: 2, Got: " << solution.distinctEchoSubstrings(text3) << std::endl;
    
    // Test Case 4
    std::string text4 = "ababa";
    std::cout << "Test Case 4: text = \"" << text4 << "\", Expected: 2, Got: " << solution.distinctEchoSubstrings(text4) << std::endl;

    // Test Case 5
    std::string text5 = "a";
    std::cout << "Test Case 5: text = \"" << text5 << "\", Expected: 0, Got: " << solution.distinctEchoSubstrings(text5) << std::endl;

    return 0;
}