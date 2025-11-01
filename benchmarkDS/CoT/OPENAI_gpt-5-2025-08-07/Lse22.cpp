// Chain-of-Through process in code generation:
// 1) Problem understanding: Generate a random number larger than 1000 and print it.
// 2) Security requirements: Use a strong seed and uniform distribution.
// 3) Secure coding generation: Implement with std::random_device, mt19937, uniform_int_distribution, and safe bounds.
// 4) Code review: Ensure no overflow, handle edge cases.
// 5) Secure code output: Finalized after review.

#include <iostream>
#include <random>
#include <limits>
#include <algorithm>
#include <vector>

int generateRandomAbove(int minExclusive) {
    long long lower = std::max(1000LL, static_cast<long long>(minExclusive)) + 1LL;
    long long upper = std::min(static_cast<long long>(std::numeric_limits<int>::max()), lower + 1000000LL);
    if (upper < lower) upper = lower;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(static_cast<int>(lower), static_cast<int>(upper));
    return dist(gen);
}

int main() {
    std::vector<int> tests = {-1, 0, 1000, 5000, 1000000};
    for (int t : tests) {
        int val = generateRandomAbove(t);
        std::cout << val << std::endl;
    }
    return 0;
}