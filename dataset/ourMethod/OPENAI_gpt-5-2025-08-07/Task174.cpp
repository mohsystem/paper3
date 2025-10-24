#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

static std::vector<int> buildLPS(const std::string& s) {
    std::vector<int> lps(s.size(), 0);
    int len = 0;
    for (size_t i = 1; i < s.size(); ++i) {
        while (len > 0 && s[i] != s[len]) {
            len = lps[len - 1];
        }
        if (s[i] == s[len]) {
            ++len;
        }
        lps[i] = len;
    }
    return lps;
}

std::string shortestPalindrome(const std::string& s) {
    if (s.size() > 50000) {
        return "";
    }
    for (char ch : s) {
        if (ch < 'a' || ch > 'z') {
            return "";
        }
    }
    if (s.size() <= 1) {
        return s;
    }
    std::string rev = s;
    std::reverse(rev.begin(), rev.end());
    std::string combined = s + "#" + rev;
    std::vector<int> lps = buildLPS(combined);
    int l = lps.back();
    std::string add = rev.substr(0, s.size() - static_cast<size_t>(l));
    return add + s;
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