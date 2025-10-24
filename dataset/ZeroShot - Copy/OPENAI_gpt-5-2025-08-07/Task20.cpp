#include <iostream>
#include <string>
#include <limits>

static bool safe_mul_ull(unsigned long long a, unsigned long long b, unsigned long long& res) {
    if (a == 0 || b == 0) { res = 0; return true; }
    if (a > std::numeric_limits<unsigned long long>::max() / b) return false;
    res = a * b;
    return true;
}

static bool safe_add_ull(unsigned long long a, unsigned long long b, unsigned long long& res) {
    if (std::numeric_limits<unsigned long long>::max() - a < b) return false;
    res = a + b; return true;
}

static bool pow_ull(unsigned long long base, int exp, unsigned long long& out) {
    out = 1;
    for (int i = 0; i < exp; ++i) {
        unsigned long long tmp;
        if (!safe_mul_ull(out, base, tmp)) return false;
        out = tmp;
    }
    return true;
}

long long digPow(long long n, int p) {
    if (n <= 0 || p <= 0) return -1;
    std::string s = std::to_string(n);
    unsigned long long sum = 0;
    int exp = p;
    for (char ch : s) {
        unsigned long long digit = static_cast<unsigned long long>(ch - '0');
        unsigned long long term;
        if (!pow_ull(digit, exp, term)) return -1;
        unsigned long long tmp;
        if (!safe_add_ull(sum, term, tmp)) return -1;
        sum = tmp;
        ++exp;
    }
    unsigned long long un = static_cast<unsigned long long>(n);
    if (un == 0) return -1;
    if (sum % un == 0) {
        unsigned long long k = sum / un;
        if (k <= static_cast<unsigned long long>(std::numeric_limits<long long>::max())) {
            return static_cast<long long>(k);
        }
    }
    return -1;
}

int main() {
    struct Test { long long n; int p; } tests[5] = {
        {89, 1},
        {92, 1},
        {695, 2},
        {46288, 3},
        {135, 1}
    };
    for (const auto& t : tests) {
        std::cout << "digPow(" << t.n << ", " << t.p << ") = " << digPow(t.n, t.p) << "\n";
    }
    return 0;
}