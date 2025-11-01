#include <iostream>
#include <array>
#include <random>

int getMonthlySales(std::random_device& rd) {
    std::uniform_int_distribution<int> dist(0, 100000); // inclusive
    return dist(rd);
}

std::array<long long, 4> computeQuarterlySales(std::random_device& rd) {
    std::array<long long, 4> quarters{0, 0, 0, 0};
    for (int month = 0; month < 12; ++month) {
        int sale = getMonthlySales(rd);
        quarters[month / 3] += sale;
    }
    return quarters;
}

int main() {
    std::random_device rd;
    for (int i = 1; i <= 5; ++i) {
        auto q = computeQuarterlySales(rd);
        std::cout << "Test case " << i << ": "
                  << "Q1: " << q[0] << ", "
                  << "Q2: " << q[1] << ", "
                  << "Q3: " << q[2] << ", "
                  << "Q4: " << q[3] << "\n";
    }
    return 0;
}