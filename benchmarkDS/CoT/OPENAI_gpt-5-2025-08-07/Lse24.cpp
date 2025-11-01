// Chain-of-Through Step 1: Problem understanding
// - Generate two random ints in [10, 1000], compute difference, print results.
// - Provide a function that accepts parameters and returns outputs.
// Chain-of-Through Step 2: Security requirements
// - Seed PRNG with std::random_device to avoid predictability.
// Chain-of-Through Step 3: Secure coding generation
// - Validate bounds, inclusive uniform distribution, absolute difference.
// Chain-of-Through Step 4: Code review
// - Use thread_local engine, handle bounds safely.
// Chain-of-Through Step 5: Secure code output

#include <iostream>
#include <random>
#include <stdexcept>
#include <cmath>

struct Result {
    int first;
    int second;
    int difference;
};

Result generateAndDiff(int min, int max) {
    if (min > max) {
        throw std::invalid_argument("min must be <= max");
    }
    static thread_local std::mt19937 rng([] {
        std::random_device rd;
        std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
        return std::mt19937(seq);
    }());
    std::uniform_int_distribution<int> dist(min, max);
    int a = dist(rng);
    int b = dist(rng);
    int diff = std::abs(a - b);
    return Result{ a, b, diff };
}

int main() {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        Result r = generateAndDiff(10, 1000);
        std::cout << "Test " << i << ": First=" << r.first
                  << ", Second=" << r.second
                  << ", Difference=" << r.difference << "\n";
    }
    return 0;
}