#include <bits/stdc++.h>
using namespace std;

bool isIsogram(const string& s) {
    int mask = 0;
    for (unsigned char ch : s) {
        char c = tolower(ch);
        if (c < 'a' || c > 'z') continue;
        int bit = c - 'a';
        if (mask & (1 << bit)) return false;
        mask |= (1 << bit);
    }
    return true;
}

int main() {
    vector<string> tests = {"Dermatoglyphics", "aba", "moOse", "", "isogram"};
    for (const auto& t : tests) {
        cout << t << " -> " << (isIsogram(t) ? "true" : "false") << "\n";
    }
    return 0;
}