#include <iostream>
#include <random>
#include <array>
#include <algorithm>
#include <stdexcept>

std::array<int, 4> generateThreeAndSum(std::mt19937& rng, int minExclusive, int maxInclusive) {
    long baseL = std::max(static_cast<long>(minExclusive) + 1L, 1001L);
    if (static_cast<long>(maxInclusive) < baseL) {
        throw std::invalid_argument("maxInclusive must be >= max(minExclusive+1, 1001)");
    }
    int base = static_cast<int>(baseL);
    std::uniform_int_distribution<int> dist(base, maxInclusive);

    int a = dist(rng);
    int b = dist(rng);
    int c = dist(rng);
    long long sum = static_cast<long long>(a) + b + c;
    if (sum > INT32_MAX || sum < INT32_MIN) {
        throw std::overflow_error("sum overflow");
    }
    return {a, b, c, static_cast<int>(sum)};
}

int main() {
    std::random_device rd;
    std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
    std::mt19937 rng(seq);

    for (int i = 0; i < 5; ++i) {
        auto r = generateThreeAndSum(rng, 1000, 1'000'000);
        std::cout << "a : " << r[0] << " b : " << r[1] << " c : " << r[2]
                  << " sum: " << r[3] << " sum2: " << r[3] << "\n";
    }
    return 0;
}