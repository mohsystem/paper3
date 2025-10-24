// Chain-of-Through process:
// 1) Problem: Compute GCD of two numbers.
// 2) Security: Avoid undefined behavior for negatives by operating on magnitudes as unsigned.
// 3) Secure generation: Iterative Euclidean algorithm on unsigned long long.
// 4) Review: Handles zeros and negatives safely; returns non-negative magnitude.
// 5) Output: Final code with 5 test cases.

#include <iostream>
#include <vector>
#include <utility>
#include <cstdint>

using ull = unsigned long long;
using ll = long long;

static ull gcd_ull(ull a, ull b) {
    while (b != 0) {
        ull t = a % b;
        a = b;
        b = t;
    }
    return a;
}

ull gcd_ll(ll x, ll y) {
    // Convert to magnitudes safely, even for LLONG_MIN
    ull a = x < 0 ? (ull)(-(unsigned long long)x) : (ull)x;
    ull b = y < 0 ? (ull)(-(unsigned long long)y) : (ull)y;
    return gcd_ull(a, b);
}

int main() {
    std::vector<std::pair<ll, ll>> tests = {
        {48, 18},
        {0, 0},
        {270, 192},
        {-24, 60},
        {13, 7}
    };
    for (const auto& p : tests) {
        ull g = gcd_ll(p.first, p.second);
        std::cout << "gcd(" << p.first << ", " << p.second << ") = " << g << "\n";
    }
    return 0;
}