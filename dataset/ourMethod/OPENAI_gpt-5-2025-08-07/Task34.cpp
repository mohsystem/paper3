#include <iostream>
#include <limits>

static unsigned int count_digits_ull(unsigned long long n) {
    if (n == 0ULL) return 1U; // Not expected per spec (positive non-zero), but safe default
    unsigned int count = 0U;
    while (n > 0ULL) {
        ++count;
        n /= 10ULL;
    }
    return count;
}

static bool mul_overflow_ull(unsigned long long a, unsigned long long b) {
    if (a == 0ULL || b == 0ULL) return false;
    return a > (std::numeric_limits<unsigned long long>::max() / b);
}

static bool add_overflow_ull(unsigned long long a, unsigned long long b) {
    return a > (std::numeric_limits<unsigned long long>::max() - b);
}

static bool pow_digit_ull(unsigned int base, unsigned int exp, unsigned long long &result) {
    // base is a single decimal digit [0..9], exp >= 1
    result = 1ULL;
    for (unsigned int i = 0; i < exp; ++i) {
        if (mul_overflow_ull(result, static_cast<unsigned long long>(base))) {
            return false; // overflow
        }
        result *= static_cast<unsigned long long>(base);
    }
    return true;
}

bool is_narcissistic(unsigned long long n) {
    if (n == 0ULL) return false; // per problem, positive non-zero integers only
    const unsigned int digits = count_digits_ull(n);

    unsigned long long sum = 0ULL;
    unsigned long long temp = n;
    while (temp > 0ULL) {
        unsigned int d = static_cast<unsigned int>(temp % 10ULL);
        temp /= 10ULL;

        unsigned long long p = 0ULL;
        if (!pow_digit_ull(d, digits, p)) {
            return false; // overflow => cannot equal n
        }
        if (add_overflow_ull(sum, p)) {
            return false; // overflow => cannot equal n
        }
        sum += p;
    }
    return sum == n;
}

int main() {
    unsigned long long tests[5] = {153ULL, 1652ULL, 7ULL, 9474ULL, 9926315ULL};
    for (int i = 0; i < 5; ++i) {
        bool res = is_narcissistic(tests[i]);
        std::cout << tests[i] << " -> " << (res ? "true" : "false") << "\n";
    }
    return 0;
}