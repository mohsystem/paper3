// Task183 - C++ implementation
// Chain-of-Through secure coding process:
// 1) Problem understanding: find largest product of any contiguous series of given span in a digit string.
// 2) Security requirements: validate inputs (negative span, non-digits, span bounds), avoid UB, use safe types.
// 3) Secure coding generation: simple O(n*span) computation; handle span==0; cautious about overflow (best-effort).
// 4) Code review: ensure indices and conditions are correct.
// 5) Secure code output: include 5 test cases printing results.

#include <iostream>
#include <string>
#include <limits>
#include <cctype>

unsigned long long largest_product(const std::string& digits, int span) {
    if (span < 0) return 0ULL;
    if (span == 0) return 1ULL;
    const size_t n = digits.size();
    if (static_cast<size_t>(span) > n) return 0ULL;

    for (char c : digits) {
        if (c < '0' || c > '9') return 0ULL;
    }

    unsigned long long maxProduct = 0ULL;
    for (size_t i = 0; i + static_cast<size_t>(span) <= n; ++i) {
        unsigned long long product = 1ULL;
        bool zeroFound = false;
        for (size_t j = i; j < i + static_cast<size_t>(span); ++j) {
            unsigned int d = static_cast<unsigned int>(digits[j] - '0');
            if (d == 0U) {
                product = 0ULL;
                zeroFound = true;
                break;
            }
            // Overflow check (best-effort): if product > max / d, cap to max
            if (product > std::numeric_limits<unsigned long long>::max() / d) {
                product = std::numeric_limits<unsigned long long>::max();
                // continue to keep deterministic
            } else {
                product *= d;
            }
        }
        if (product > maxProduct) {
            maxProduct = product;
        }
        if (maxProduct == std::numeric_limits<unsigned long long>::max()) break;
    }
    return maxProduct;
}

int main() {
    // 5 test cases
    std::pair<std::string, int> tests[5] = {
        {"63915", 3},
        {"1234567890", 2},
        {"0000", 2},
        {"99999", 5},
        {"", 0}
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << (i + 1) << ": largest_product(\"" << tests[i].first << "\", " << tests[i].second
                  << ") = " << largest_product(tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}