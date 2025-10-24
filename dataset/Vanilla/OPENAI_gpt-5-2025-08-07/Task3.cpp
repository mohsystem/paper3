#include <bits/stdc++.h>
using namespace std;

bool isPangram(const string& s) {
    array<bool, 26> seen{};
    int count = 0;
    for (unsigned char ch : s) {
        if (ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
        if (ch >= 'a' && ch <= 'z') {
            int idx = ch - 'a';
            if (!seen[idx]) {
                seen[idx] = true;
                count++;
                if (count == 26) return true;
            }
        }
    }
    return count == 26;
}

int main() {
    vector<string> tests = {
        "The quick brown fox jumps over the lazy dog",
        "Sphinx of black quartz, judge my vow",
        "Hello, World!",
        "",
        "Pack my box with five dozen liquor jugs."
    };
    for (const auto& t : tests) {
        cout << (isPangram(t) ? "true" : "false") << endl;
    }
    return 0;
}