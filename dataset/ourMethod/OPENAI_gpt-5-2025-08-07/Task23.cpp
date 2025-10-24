#include <iostream>
#include <cstdint>
#include <limits>

static unsigned long long isqrt_ull(unsigned long long x) {
    unsigned long long left = 0, right = (x < std::numeric_limits<unsigned long long>::max() ? x : std::numeric_limits<unsigned long long>::max());
    if (right > 4294967295ULL) right = 4294967295ULL; // sqrt(2^64-1) < 2^32
    unsigned long long ans = 0;
    while (left <= right) {
        unsigned long long mid = left + ((right - left) >> 1);
        unsigned long long sq = mid * mid;
        if (sq == x) return mid;
        if (sq < x) {
            ans = mid;
            left = mid + 1;
        } else {
            if (mid == 0) break;
            right = mid - 1;
        }
    }
    return ans;
}

long long findNb(unsigned long long m) {
    // Validate input
    // For m == 0, n = 0 satisfies the equation.
    if (m == 0ULL) return 0;
    // Sum of cubes equals m => (n(n+1)/2)^2 = m
    unsigned long long k = isqrt_ull(m);
    if (k * k != m) return -1;
    // Solve n(n+1)/2 = k with integer arithmetic
    unsigned long long d = 1ULL + 8ULL * k; // discriminant
    unsigned long long s = isqrt_ull(d);
    if (s * s != d) return -1;
    if ((s & 1ULL) == 0ULL) return -1; // s must be odd
    unsigned long long n = (s - 1ULL) / 2ULL;
    if (n > 0ULL && (n * (n + 1ULL)) / 2ULL == k) return static_cast<long long>(n);
    if (n == 0ULL && k == 0ULL) return 0;
    return -1;
}

int main() {
    // 5 test cases
    unsigned long long tests[5];
    tests[0] = 1071225ULL;              // expected 45
    tests[1] = 91716553919377ULL;       // expected -1
    tests[2] = 1ULL;                    // expected 1
    tests[3] = 9ULL;                    // expected 2
    // Construct m for n = 2000 -> m = (n(n+1)/2)^2
    unsigned long long n = 2000ULL;
    unsigned long long s = (n * (n + 1ULL)) / 2ULL;
    tests[4] = s * s;                   // expected 2000

    for (int i = 0; i < 5; ++i) {
        long long res = findNb(tests[i]);
        std::cout << "findNb(" << tests[i] << ") = " << res << "\n";
    }
    return 0;
}