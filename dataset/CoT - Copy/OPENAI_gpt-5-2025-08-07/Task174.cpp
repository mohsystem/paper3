// Step 1-5: Secure, efficient implementation using KMP to find longest palindromic prefix.
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

std::string shortestPalindrome(const std::string& s) {
    // Handle empty and single char
    if (s.size() <= 1) return s;
    std::string rev = s;
    std::reverse(rev.begin(), rev.end());
    std::string combined = s + "#" + rev;

    // KMP prefix function
    std::vector<int> pi(combined.size(), 0);
    for (size_t i = 1; i < combined.size(); ++i) {
        int j = pi[i - 1];
        while (j > 0 && combined[i] != combined[j]) j = pi[j - 1];
        if (combined[i] == combined[j]) ++j;
        pi[i] = j;
    }
    int L = pi.back();
    std::string prefixToAdd = s.substr(L);
    std::reverse(prefixToAdd.begin(), prefixToAdd.end());
    return prefixToAdd + s;
}

int main() {
    std::vector<std::string> tests = {
        "aacecaaa",
        "abcd",
        "",
        "a",
        "aaab"
    };
    for (const auto& t : tests) {
        std::string res = shortestPalindrome(t);
        std::cout << "Input: \"" << t << "\" -> Output: \"" << res << "\"\n";
    }
    return 0;
}