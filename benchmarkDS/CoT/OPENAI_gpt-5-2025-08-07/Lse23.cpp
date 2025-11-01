// Chain-of-Through process in code generation:
// 1. Problem understanding: Generate 3 random integers > 1000, print them, compute sum and print sum and sum2.
// 2. Security requirements: Use high-quality randomness from std::random_device to seed engine; validate ranges.
// 3. Secure coding generation: Use uniform_int_distribution with correct bounds (> 1000).
// 4. Code review: Checked for exceptions on invalid input and correct formatting.
// 5. Secure code output: Final program with 5 test cases in main.

#include <iostream>
#include <random>
#include <stdexcept>

struct Result {
    int a, b, c, sum, sum2;
};

Result generateAndSum(int minExclusive, int maxInclusive, std::mt19937& rng) {
    if (maxInclusive <= minExclusive) {
        throw std::invalid_argument("maxInclusive must be greater than minExclusive");
    }
    std::uniform_int_distribution<int> dist(minExclusive + 1, maxInclusive);
    Result r{};
    r.a = dist(rng);
    r.b = dist(rng);
    r.c = dist(rng);
    r.sum = r.a + r.b + r.c;
    r.sum2 = r.sum;
    return r;
}

int main() {
    std::random_device rd;
    std::seed_seq seed{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
    std::mt19937 rng(seed);

    int minExclusive = 1000;
    int maxInclusive = 9999;

    for (int i = 0; i < 5; ++i) {
        Result r = generateAndSum(minExclusive, maxInclusive, rng);
        std::cout << "a : " << r.a << " b : " << r.b << " c : " << r.c
                  << " sum: " << r.sum << " sum2: " << r.sum2 << std::endl;
    }
    return 0;
}