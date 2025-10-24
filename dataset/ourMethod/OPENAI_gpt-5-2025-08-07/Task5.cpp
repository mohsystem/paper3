#include <iostream>
#include <vector>
#include <climits>

int persistence(unsigned long long num) {
    if (num == 0ULL) {
        // Invalid input per problem statement (expects positive integer)
        return -1;
    }
    if (num < 10ULL) {
        return 0;
    }

    int count = 0;
    while (num >= 10ULL) {
        unsigned long long product = 1ULL;
        unsigned long long temp = num;

        while (temp > 0ULL) {
            unsigned int d = static_cast<unsigned int>(temp % 10ULL);
            if (d != 0U && product > (ULLONG_MAX / d)) {
                // Overflow would occur; fail closed
                return -1;
            }
            product *= d;
            temp /= 10ULL;
        }
        ++count;
        num = product;
    }
    return count;
}

int main() {
    // 5 test cases
    std::vector<unsigned long long> tests = {
        39ULL,         // expected 3
        999ULL,        // expected 4
        4ULL,          // expected 0
        25ULL,         // expected 2
        77ULL          // expected 4
    };

    for (auto n : tests) {
        int result = persistence(n);
        std::cout << "persistence(" << n << ") = " << result << std::endl;
    }

    return 0;
}