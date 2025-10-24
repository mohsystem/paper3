#include <bits/stdc++.h>
using namespace std;

class Task174 {
public:
    static string shortestPalindrome(const string& s) {
        if (s.empty()) return s;
        string rev = s;
        reverse(rev.begin(), rev.end());
        string combined = s + "#" + rev;
        vector<int> lps = buildLPS(combined);
        int prefixLen = lps.back();
        string suffix = s.substr(prefixLen);
        string res = string(suffix.rbegin(), suffix.rend()) + s;
        return res;
    }
private:
    static vector<int> buildLPS(const string& str) {
        vector<int> lps(str.size(), 0);
        int len = 0;
        for (size_t i = 1; i < str.size(); ++i) {
            while (len > 0 && str[i] != str[len]) {
                len = lps[len - 1];
            }
            if (str[i] == str[len]) ++len;
            lps[i] = len;
        }
        return lps;
    }
};

int main() {
    vector<string> tests = {
        "aacecaaa",
        "abcd",
        "",
        "a",
        "aaab"
    };
    for (const auto& t : tests) {
        cout << Task174::shortestPalindrome(t) << "\n";
    }
    return 0;
}