#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <utility>

struct PairHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        // A common way to combine hashes
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

int search(const std::string& s, int L) {
    int n = s.length();
    if (L == 0) return 0;
    if (L >= n) return -1;

    long long base1 = 26;
    long long base2 = 31;
    long long mod1 = 1e9 + 7;
    long long mod2 = 1e9 + 9;

    long long h1 = 1;
    long long h2 = 1;
    for (int i = 0; i < L - 1; ++i) {
        h1 = (h1 * base1) % mod1;
        h2 = (h2 * base2) % mod2;
    }

    long long currentHash1 = 0;
    long long currentHash2 = 0;
    for (int i = 0; i < L; ++i) {
        currentHash1 = (currentHash1 * base1 + (s[i] - 'a')) % mod1;
        currentHash2 = (currentHash2 * base2 + (s[i] - 'a')) % mod2;
    }

    std::unordered_set<std::pair<long long, long long>, PairHash> seen;
    seen.insert({currentHash1, currentHash2});

    for (int i = 1; i <= n - L; ++i) {
        long long prevCharVal = s[i - 1] - 'a';
        long long nextCharVal = s[i + L - 1] - 'a';

        currentHash1 = (currentHash1 - (prevCharVal * h1) % mod1 + mod1) % mod1;
        currentHash1 = (currentHash1 * base1) % mod1;
        currentHash1 = (currentHash1 + nextCharVal) % mod1;
        
        currentHash2 = (currentHash2 - (prevCharVal * h2) % mod2 + mod2) % mod2;
        currentHash2 = (currentHash2 * base2) % mod2;
        currentHash2 = (currentHash2 + nextCharVal) % mod2;

        if (seen.count({currentHash1, currentHash2})) {
            return i;
        }
        seen.insert({currentHash1, currentHash2});
    }
    return -1;
}

std::string longestDupSubstring(std::string s) {
    int n = s.length();
    int low = 1, high = n;
    int bestLen = 0;
    int start = -1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (mid == 0) {
            low = mid + 1;
            continue;
        }
        int foundStart = search(s, mid);
        
        if (foundStart != -1) {
            bestLen = mid;
            start = foundStart;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (start != -1) {
        return s.substr(start, bestLen);
    }
    return "";
}

int main() {
    std::vector<std::string> testCases = {
        "banana",
        "abcd",
        "ababa",
        "mississippi",
        "aaaaaaaaaa"
    };
    std::vector<std::string> expectedResults = {
        "ana",
        "",
        "aba",
        "issi",
        "aaaaaaaaa"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::string s = testCases[i];
        std::string result = longestDupSubstring(s);
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Input: s = \"" << s << "\"" << std::endl;
        std::cout << "Output: \"" << result << "\"" << std::endl;
        std::cout << "Expected: \"" << expectedResults[i] << "\"" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}