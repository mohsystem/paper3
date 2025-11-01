#include <iostream>
#include <array>
#include <random>

int getMonthlySales() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 100000);
    return dist(rng);
}

std::array<int, 12> generateYearlySales() {
    std::array<int, 12> monthly{};
    for (int i = 0; i < 12; ++i) {
        monthly[i] = getMonthlySales();
    }
    return monthly;
}

std::array<long long, 4> quarterlySums(const std::array<int, 12>& monthly) {
    std::array<long long, 4> quarters{0, 0, 0, 0};
    for (int i = 0; i < 12; ++i) {
        quarters[i / 3] += monthly[i];
    }
    return quarters;
}

void printQuarterly(const std::array<long long, 4>& quarters, int caseNo) {
    std::cout << "Test case " << caseNo << ":\n";
    for (int i = 0; i < 4; ++i) {
        std::cout << "Q" << (i + 1) << ": " << quarters[i] << "\n";
    }
}

int main() {
    for (int t = 1; t <= 5; ++t) {
        auto monthly = generateYearlySales();
        auto q = quarterlySums(monthly);
        printQuarterly(q, t);
    }
    return 0;
}