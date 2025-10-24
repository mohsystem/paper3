#include <iostream>
#include <string>
#include <climits>

static unsigned __int128 u128_max_value() {
    return (unsigned __int128)(-1);
}

static unsigned __int128 safe_mul_u128(unsigned __int128 a, unsigned __int128 b, bool &overflow) {
    unsigned __int128 maxv = u128_max_value();
    if (a != 0 && b > maxv / a) {
        overflow = true;
        return maxv;
    }
    return a * b;
}

static unsigned __int128 ipow_u128(unsigned int base, unsigned int exp, bool &overflow) {
    if (base == 0) return 0;
    unsigned __int128 result = 1;
    unsigned __int128 b = base;
    unsigned int e = exp;
    // Exponentiation by squaring with overflow checks
    while (e > 0 && !overflow) {
        if (e & 1u) {
            result = safe_mul_u128(result, b, overflow);
        }
        e >>= 1u;
        if (e > 0) {
            b = safe_mul_u128(b, b, overflow);
        }
    }
    return overflow ? u128_max_value() : result;
}

long long digPow(unsigned long long n, unsigned int p) {
    if (n == 0ull || p == 0u) return -1;
    std::string s = std::to_string(n);

    bool overflow = false;
    unsigned __int128 sum = 0;
    unsigned __int128 maxv = u128_max_value();

    for (size_t i = 0; i < s.size(); ++i) {
        unsigned int digit = static_cast<unsigned int>(s[i] - '0');
        unsigned int exp = p + static_cast<unsigned int>(i);
        unsigned __int128 term = ipow_u128(digit, exp, overflow);
        if (overflow) return -1;
        if (sum > maxv - term) {
            overflow = true;
            return -1;
        }
        sum += term;
    }

    // sum = n * k  => k = sum / n if divisible
    if (n == 0ull) return -1; // defensive, already checked
    unsigned long long rem = static_cast<unsigned long long>(sum % n);
    if (rem != 0ull) return -1;

    unsigned __int128 k128 = sum / n;

    if (k128 == 0) return -1;
    unsigned __int128 llmax = static_cast<unsigned __int128>(LLONG_MAX);
    if (k128 > llmax) return -1;

    return static_cast<long long>(k128);
}

int main() {
    struct Test { unsigned long long n; unsigned int p; };
    Test tests[5] = {
        {89ull, 1u},
        {92ull, 1u},
        {695ull, 2u},
        {46288ull, 3u},
        {1ull, 1u}
    };

    for (const auto &t : tests) {
        long long k = digPow(t.n, t.p);
        std::cout << "n=" << t.n << ", p=" << t.p << " => k=" << k << "\n";
    }
    return 0;
}