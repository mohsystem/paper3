// Chain-of-Through process:
// 1) Problem understanding: Generate 12 monthly sales in [0,100000], sum per quarter, print totals.
// 2) Security requirements: Use local RNG (mt19937) with explicit seed; no global state; no unsafe I/O.
// 3) Secure coding generation: Strong typing, bounds checks, no overflows (safe int range).
// 4) Code review: Deterministic tests; no dynamic allocation risks.
// 5) Secure code output: Clean, reproducible, and safe.

#include <iostream>
#include <random>
#include <array>
#include <cstdint>

int getMonthlySales(std::mt19937& rng, std::uniform_int_distribution<int>& dist) {
    return dist(rng);
}

std::array<int, 4> computeQuarterlySales(uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 100000);
    std::array<int, 4> quarters{0, 0, 0, 0};
    for (int month = 0; month < 12; ++month) {
        int sale = getMonthlySales(rng, dist);
        quarters[static_cast<size_t>(month / 3)] += sale;
    }
    return quarters;
}

void printQuarterResults(uint32_t seed, const std::array<int,4>& quarters) {
    std::cout << "Seed " << seed << " quarterly totals:\n";
    for (size_t i = 0; i < quarters.size(); ++i) {
        std::cout << "  Q" << (i + 1) << ": " << quarters[i] << "\n";
    }
}

int main() {
    uint32_t seeds[5] = {0u, 1u, 42u, 2025u, 999999u};
    for (uint32_t s : seeds) {
        auto q = computeQuarterlySales(s);
        printQuarterResults(s, q);
    }
    return 0;
}