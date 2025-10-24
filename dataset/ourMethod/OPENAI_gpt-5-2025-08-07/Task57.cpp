#include <iostream>
#include <vector>
#include <cstdint>
#include <limits>

enum class FactError {
    None = 0,
    NegativeInput = 1,
    Overflow = 2
};

struct FactorialResult {
    bool ok;
    uint64_t value;
    FactError error;
};

FactorialResult factorial(long long n) {
    FactorialResult res;
    if (n < 0) {
        res.ok = false;
        res.value = 0;
        res.error = FactError::NegativeInput;
        return res;
    }

    uint64_t result = 1;
    for (long long i = 2; i <= n; ++i) {
        uint64_t ui = static_cast<uint64_t>(i);
        if (result > std::numeric_limits<uint64_t>::max() / ui) {
            res.ok = false;
            res.value = 0;
            res.error = FactError::Overflow;
            return res;
        }
        result *= ui;
    }

    res.ok = true;
    res.value = result;
    res.error = FactError::None;
    return res;
}

int main() {
    std::vector<long long> tests = {0LL, 1LL, 5LL, 20LL, 21LL};

    for (long long n : tests) {
        FactorialResult r = factorial(n);
        if (r.ok) {
            std::cout << "factorial(" << n << ") = " << r.value << "\n";
        } else {
            if (r.error == FactError::NegativeInput) {
                std::cout << "Error for n=" << n << ": input must be a non-negative integer.\n";
            } else if (r.error == FactError::Overflow) {
                std::cout << "Error for n=" << n << ": overflow (result does not fit in 64-bit unsigned integer).\n";
            } else {
                std::cout << "Error for n=" << n << ": unknown error.\n";
            }
        }
    }

    return 0;
}