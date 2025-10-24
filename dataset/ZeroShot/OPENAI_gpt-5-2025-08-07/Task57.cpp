#include <iostream>
#include <limits>
#include <vector>
#include <cstdint>

uint64_t factorial_u64(unsigned int n, bool& overflow) {
    overflow = false;
    uint64_t result = 1;
    for (unsigned int i = 2; i <= n; ++i) {
        if (result > std::numeric_limits<uint64_t>::max() / i) {
            overflow = true;
            return 0;
        }
        result *= i;
    }
    return result;
}

int main() {
    std::vector<unsigned int> tests = {0u, 1u, 5u, 10u, 20u};
    for (auto n : tests) {
        bool overflow = false;
        uint64_t res = factorial_u64(n, overflow);
        if (overflow) {
            std::cout << "factorial(" << n << ") overflow\n";
        } else {
            std::cout << "factorial(" << n << ") = " << res << "\n";
        }
    }
    return 0;
}