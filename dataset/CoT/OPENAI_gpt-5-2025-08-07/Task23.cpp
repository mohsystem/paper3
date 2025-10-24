// Chain-of-Through process in code generation:
// 1) Problem understanding: findNb(m) -> n if sum_{k=1..n} k^3 == m, else -1.
// 2) Security requirements: Avoid FP inaccuracies and overflow using unsigned __int128.
// 3) Secure coding generation: Binary search on n with monotonic f(n).
// 4) Code review: Strict types, bounds checks, no UB.
// 5) Secure code output: Final code with 5 test cases.
#include <bits/stdc++.h>
using namespace std;

long long findNb(unsigned long long m) {
    if (m == 0ULL) return 0;
    auto f = [](unsigned long long n) -> unsigned __int128 {
        unsigned __int128 nn = n;
        unsigned __int128 t = nn * (nn + 1u) / 2u;
        return t * t;
    };
    unsigned long long low = 0, high = 1;
    while (f(high) < (unsigned __int128)m && high < (numeric_limits<unsigned long long>::max() >> 1)) {
        high <<= 1;
    }
    long long ans = -1;
    while (low <= high) {
        unsigned long long mid = low + ((high - low) >> 1);
        unsigned __int128 val = f(mid);
        if (val == (unsigned __int128)m) {
            ans = (long long)mid;
            break;
        } else if (val < (unsigned __int128)m) {
            low = mid + 1;
        } else {
            if (mid == 0) break;
            high = mid - 1;
        }
    }
    return ans;
}

int main() {
    vector<unsigned long long> tests = {
        1071225ULL,           // -> 45
        91716553919377ULL,    // -> -1
        4183059834009ULL,     // -> 2022
        24723578342962ULL,    // -> -1
        135440716410000ULL    // -> 4824
    };
    for (auto m : tests) {
        cout << findNb(m) << '\n';
    }
    return 0;
}