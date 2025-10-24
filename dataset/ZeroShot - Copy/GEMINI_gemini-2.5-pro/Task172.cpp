#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <utility>

long long get_hash(const std::vector<long long>& h, const std::vector<long long>& p_pow, long long M, int i, int j) {
    int len = j - i + 1;
    long long term = (h[i] * p_pow[len]) % M;
    long long raw_hash = h[j + 1] - term;
    return (raw_hash % M + M) % M;
}

int distinctEchoSubstrings(std::string text) {
    using ll = long long;
    ll P1 = 31, M1 = 1e9 + 7;
    ll P2 = 37, M2 = 1e9 + 9;
    int n = text.length();

    if (n == 0) return 0;

    std::vector<ll> p_pow1(n + 1);
    std::vector<ll> h1(n + 1, 0);
    std::vector<ll> p_pow2(n + 1);
    std::vector<ll> h2(n + 1, 0);

    p_pow1[0] = 1;
    p_pow2[0] = 1;

    for (int i = 0; i < n; ++i) {
        p_pow1[i+1] = (p_pow1[i] * P1) % M1;
        h1[i+1] = (h1[i] * P1 + (text[i] - 'a' + 1)) % M1;
        p_pow2[i+1] = (p_pow2[i] * P2) % M2;
        h2[i+1] = (h2[i] * P2 + (text[i] - 'a' + 1)) % M2;
    }

    std::set<std::pair<ll, ll>> found_hashes;
    for (int l = 1; l <= n / 2; ++l) { // l is the length of the repeating part 'a'
        for (int i = 0; i <= n - 2 * l; ++i) { // i is the start of 'a+a'
            int j = i + l; // j is the start of the second 'a'
            
            // Compare text[i...i+l-1] and text[j...j+l-1] using double hashing
            ll h1_a = get_hash(h1, p_pow1, M1, i, i + l - 1);
            ll h1_b = get_hash(h1, p_pow1, M1, j, j + l - 1);
            
            if (h1_a == h1_b) {
                ll h2_a = get_hash(h2, p_pow2, M2, i, i + l - 1);
                ll h2_b = get_hash(h2, p_pow2, M2, j, j + l - 1);
                
                if (h2_a == h2_b) {
                    // Strings are very likely equal. Add the hash of the 'a+a' substring.
                    ll hash_aa_1 = get_hash(h1, p_pow1, M1, i, i + 2*l - 1);
                    ll hash_aa_2 = get_hash(h2, p_pow2, M2, i, i + 2*l - 1);
                    found_hashes.insert({hash_aa_1, hash_aa_2});
                }
            }
        }
    }
    return found_hashes.size();
}

int main() {
    std::string test_cases[] = {
        "abcabcabc",       // Expected: 3
        "leetcodeleetcode",// Expected: 2
        "a",               // Expected: 0
        "aaaaa",           // Expected: 2
        "abacaba"          // Expected: 0
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::string text = test_cases[i];
        int result = distinctEchoSubstrings(text);
        std::cout << "Input: text = \"" << text << "\"" << std::endl;
        std::cout << "Output: " << result << std::endl << std::endl;
    }

    return 0;
}