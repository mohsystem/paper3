#include <array>
#include <cstdint>
#include <iostream>
#include <random>

int getMonthlySales(std::mt19937& rng) {
    static std::uniform_int_distribution<int> dist(0, 100000);
    return dist(rng);
}

std::array<long long, 4> computeQuarterlySales(std::mt19937& rng) {
    std::array<long long, 4> quarters{0, 0, 0, 0};
    for (int month = 0; month < 12; ++month) {
        int sale = getMonthlySales(rng);
        quarters[month / 3] += static_cast<long long>(sale);
    }
    return quarters;
}

int main() {
    for (int t = 1; t <= 5; ++t) {
        std::random_device rd;
        std::mt19937 rng(rd());
        auto q = computeQuarterlySales(rng);
        std::cout << "Test " << t << " - Quarterly sales: "
                  << "Q1=" << q[0] << ", Q2=" << q[1]
                  << ", Q3=" << q[2] << ", Q4=" << q[3] << '\n';
    }
    return 0;
}