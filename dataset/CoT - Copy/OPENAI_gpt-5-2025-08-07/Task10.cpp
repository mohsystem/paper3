// Chain-of-Through process:
// 1) Problem: Return inclusive sum between two integers a and b (unordered); if equal, return a.
// 2) Security: Use long long to reduce overflow, and divide the even factor before multiplication.
// 3) Secure coding: O(1) arithmetic series formula; no loops over wide ranges.
// 4) Review: Integer division is applied to an even factor to avoid precision loss; handles negatives.
// 5) Secure output: Function with 5 test cases in main.

#include <iostream>
#include <algorithm>

long long get_sum(long long a, long long b) {
    if (a == b) return a;
    long long m = std::min(a, b);
    long long n = std::max(a, b);
    long long count = n - m + 1LL;
    long long s = m + n;
    if ((count & 1LL) == 0LL) {
        return (count / 2LL) * s;
    } else {
        return count * (s / 2LL);
    }
}

int main() {
    long long tests[][2] = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };
    for (auto& t : tests) {
        std::cout << get_sum(t[0], t[1]) << '\n';
    }
    return 0;
}