#include <bits/stdc++.h>
using namespace std;

string shortestPalindrome(const string& s) {
    if (s.size() <= 1) return s;
    string rev = string(s.rbegin(), s.rend());
    string pattern = s + "#" + rev;
    vector<int> lps(pattern.size(), 0);
    int len = 0;
    for (size_t i = 1; i < pattern.size();) {
        if (pattern[i] == pattern[len]) {
            lps[i++] = ++len;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i++] = 0;
        }
    }
    int palLen = lps.back();
    string suffix = s.substr(palLen);
    reverse(suffix.begin(), suffix.end());
    return suffix + s;
}

int main() {
    vector<string> tests = {
        "aacecaaa",
        "abcd",
        "",
        "a",
        "abbacd"
    };
    for (const auto& t : tests) {
        cout << shortestPalindrome(t) << "\n";
    }
    return 0;
}