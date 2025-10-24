// Task168 - C++ implementation
// Chain-of-Through process in code generation:
// 1) Problem: Shift lowercase letters per offsets with wrap-around.
// 2) Security: Avoid out-of-range access; handle negatives safely.
// 3) Secure coding: Use size checks, modulo normalization.
// 4) Review: Ensure correct handling when arr shorter than string.
// 5) Output: Final tested function and main with 5 cases.

#include <bits/stdc++.h>
using namespace std;

string tweakLetters(const string& s, const vector<int>& arr) {
    string res;
    res.reserve(s.size());
    size_t n = s.size();
    size_t m = arr.size();
    for (size_t i = 0; i < n; ++i) {
        char ch = s[i];
        if (ch >= 'a' && ch <= 'z' && i < m) {
            int base = ch - 'a';
            int shift = arr[i] % 26;
            if (shift < 0) shift += 26;
            int ni = (base + shift) % 26;
            res.push_back(static_cast<char>('a' + ni));
        } else if (ch >= 'a' && ch <= 'z') {
            res.push_back(ch);
        } else {
            res.push_back(ch);
        }
    }
    return res;
}

int main() {
    cout << tweakLetters("apple", {0, 1, -1, 0, -1}) << "\n"; // aqold
    cout << tweakLetters("many", {0, 0, 0, -1}) << "\n";      // manx
    cout << tweakLetters("rhino", {1, 1, 1, 1, 1}) << "\n";   // sijop
    cout << tweakLetters("zzz", {1, 1, 1}) << "\n";           // aaa
    cout << tweakLetters("abc", {-1, -1, -1}) << "\n";        // zab
    return 0;
}