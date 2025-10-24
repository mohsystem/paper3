#include <bits/stdc++.h>
using namespace std;

long long largestProduct(const string& s, int span) {
    if (span < 0) return 0;
    if (span == 0) return 1;
    if (span > (int)s.size()) return 0;
    for (char c : s) if (c < '0' || c > '9') return 0;
    long long best = 0;
    for (int i = 0; i + span <= (int)s.size(); ++i) {
        long long prod = 1;
        for (int j = i; j < i + span; ++j) {
            prod *= (s[j] - '0');
            if (prod == 0) break;
        }
        if (prod > best) best = prod;
    }
    return best;
}

int main() {
    cout << largestProduct("63915", 3) << '\n';  // 162
    cout << largestProduct("12345", 2) << '\n';  // 20
    cout << largestProduct("000123", 2) << '\n'; // 6
    cout << largestProduct("", 0) << '\n';       // 1
    cout << largestProduct("99999", 5) << '\n';  // 59049
    return 0;
}