#include <iostream>
#include <limits>
#include <cstdint>

static inline unsigned long long umag(long long x) {
    if (x >= 0) return static_cast<unsigned long long>(x);
    if (x == std::numeric_limits<long long>::min()) {
        return static_cast<unsigned long long>(std::numeric_limits<long long>::max()) + 1ULL;
    }
    return static_cast<unsigned long long>(-x);
}

long long gcd_ll(long long a, long long b) {
    unsigned long long x = umag(a);
    unsigned long long y = umag(b);
    if (x == 0ULL && y == 0ULL) return 0LL;
    while (y != 0ULL) {
        unsigned long long r = x % y;
        x = y;
        y = r;
    }
    if (x > static_cast<unsigned long long>(std::numeric_limits<long long>::max())) {
        return std::numeric_limits<long long>::min(); // Sentinel for 2^63
    }
    return static_cast<long long>(x);
}

int main() {
    long long tests[5][2] = {
        {48, 18},
        {0, 0},
        {-48, 18},
        {270, 192},
        {1234567890LL, 9876543210LL}
    };
    for (auto &t : tests) {
        long long res = gcd_ll(t[0], t[1]);
        std::cout << "gcd(" << t[0] << ", " << t[1] << ") = " << res << "\n";
    }
    return 0;
}