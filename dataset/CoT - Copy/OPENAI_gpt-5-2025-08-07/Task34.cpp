// Chain-of-Through process:
// 1) Problem understanding: Check if n equals sum of each decimal digit raised to count of digits.
// 2) Security requirements: Avoid floating-point pow; use integer exponentiation.
// 3) Secure coding generation: Guard against overflow by early comparison with n before adding.
// 4) Code review: Ensure no undefined behavior and safe arithmetic.
// 5) Secure code output: Function returns bool; includes 5 test cases.

#include <iostream>
#include <string>
#include <vector>

static unsigned long long ipow_ull(unsigned int base, unsigned int exp) {
    unsigned long long result = 1ULL;
    unsigned long long b = base;
    unsigned int e = exp;
    while (e > 0) {
        if (e & 1U) result *= b;
        b *= b;
        e >>= 1U;
    }
    return result;
}

bool narcissistic(unsigned long long n) {
    if (n == 0ULL) return false; // Only positive non-zero integers
    std::string s = std::to_string(n);
    unsigned int k = static_cast<unsigned int>(s.size());
    unsigned long long sum = 0ULL;

    for (char c : s) {
        unsigned int d = static_cast<unsigned int>(c - '0');
        unsigned long long term = ipow_ull(d, k);
        if (term > n) return false;
        if (sum > n - term) return false;
        sum += term;
    }
    return sum == n;
}

int main() {
    std::vector<unsigned long long> tests = {153ULL, 1652ULL, 9474ULL, 7ULL, 10ULL};
    std::cout << std::boolalpha;
    for (auto t : tests) {
        std::cout << "n=" << t << " -> " << narcissistic(t) << "\n";
    }
    return 0;
}