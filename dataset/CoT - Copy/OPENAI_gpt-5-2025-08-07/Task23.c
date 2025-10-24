// Chain-of-Through process in code generation:
// 1) Problem understanding: findNb(m) -> n if sum_{k=1..n} k^3 == m, else -1.
// 2) Security requirements: Use unsigned __int128 for safe multiplication, avoid FP.
// 3) Secure coding generation: Binary search with monotonic f(n).
// 4) Code review: No undefined behavior, careful with bounds.
// 5) Secure code output: Final code with 5 tests.
#include <stdio.h>
#include <limits.h>

static unsigned __int128 f_u128(unsigned long long n) {
    unsigned __int128 nn = n;
    unsigned __int128 t = nn * (nn + 1u) / 2u;
    return t * t;
}

long long findNb(unsigned long long m) {
    if (m == 0ULL) return 0;
    unsigned long long low = 0ULL, high = 1ULL;
    while (f_u128(high) < (unsigned __int128)m && high < (ULLONG_MAX >> 1)) {
        high <<= 1;
    }
    long long ans = -1;
    while (low <= high) {
        unsigned long long mid = low + ((high - low) >> 1);
        unsigned __int128 val = f_u128(mid);
        if (val == (unsigned __int128)m) {
            ans = (long long)mid;
            break;
        } else if (val < (unsigned __int128)m) {
            low = mid + 1;
        } else {
            if (mid == 0ULL) break;
            high = mid - 1;
        }
    }
    return ans;
}

int main() {
    unsigned long long tests[5] = {
        1071225ULL,           // -> 45
        91716553919377ULL,    // -> -1
        4183059834009ULL,     // -> 2022
        24723578342962ULL,    // -> -1
        135440716410000ULL    // -> 4824
    };
    for (int i = 0; i < 5; ++i) {
        printf("%lld\n", findNb(tests[i]));
    }
    return 0;
}