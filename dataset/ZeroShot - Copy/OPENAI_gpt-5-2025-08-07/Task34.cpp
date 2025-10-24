#include <iostream>
#include <limits>
#include <vector>

static unsigned int countDigits(unsigned long long n) {
    unsigned int cnt = 0;
    do {
        ++cnt;
        n /= 10ULL;
    } while (n > 0ULL);
    return cnt;
}

static bool safePowULL(unsigned long long base, unsigned int exp, unsigned long long &out) {
    unsigned long long result = 1ULL;
    for (unsigned int i = 0; i < exp; ++i) {
        if (base != 0ULL && result > std::numeric_limits<unsigned long long>::max() / base) {
            return false; // overflow
        }
        result *= base;
    }
    out = result;
    return true;
}

bool isNarcissistic(unsigned long long n) {
    if (n == 0ULL) {
        // Typically 0 is considered Armstrong (0^1 = 0). But per prompt, positive non-zero inputs are expected.
        return true;
    }
    unsigned int digits = countDigits(n);
    unsigned long long sum = 0ULL;
    unsigned long long t = n;
    while (t > 0ULL) {
        unsigned long long d = t % 10ULL;
        unsigned long long term = 0ULL;
        if (!safePowULL(d, digits, term)) {
            return false; // term overflow implies sum cannot equal n (which is within ULL range)
        }
        if (sum > std::numeric_limits<unsigned long long>::max() - term) {
            return false; // sum overflow cannot equal n
        }
        sum += term;
        t /= 10ULL;
    }
    return sum == n;
}

int main() {
    std::vector<unsigned long long> tests = {153ULL, 1652ULL, 9474ULL, 370ULL, 9926315ULL};
    std::cout.setf(std::ios::boolalpha);
    for (auto t : tests) {
        std::cout << isNarcissistic(t) << std::endl;
    }
    return 0;
}