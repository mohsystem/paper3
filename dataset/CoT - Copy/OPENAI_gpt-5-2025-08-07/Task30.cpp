#include <bits/stdc++.h>
using namespace std;

/*
 Chain-of-Through Process:
 1) Problem understanding: Unite letters from two strings, deduplicate, sort ascending, return as string.
 2) Security requirements: Process only alphabetic a-z characters safely; avoid unsafe pointer ops.
 3) Secure coding generation: Use fixed-size array for presence, bounds-checked indices.
 4) Code review: Ensure only indices 0..25 are used; handle empty inputs.
 5) Secure code output: Return sorted distinct lowercase letters.
*/

string longest(const string& s1, const string& s2) {
    array<bool, 26> present{};
    auto add = [&](const string& s) {
        for (unsigned char uc : s) {
            char c = (char)uc;
            if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
            if (c >= 'a' && c <= 'z') {
                present[c - 'a'] = true;
            }
        }
    };
    add(s1);
    add(s2);

    string out;
    out.reserve(26);
    for (int i = 0; i < 26; ++i) {
        if (present[i]) out.push_back(char('a' + i));
    }
    return out;
}

int main() {
    vector<pair<string,string>> tests = {
        {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
        {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
        {"abc", "def"},
        {"zzz", "zzzz"},
        {"", "abc"}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << ": " << longest(tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}