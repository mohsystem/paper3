#include <iostream>
#include <tuple>
#include <random>

std::tuple<int,int,int> generateAndDiff(int min, int max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(min, max);
    int a = dist(rng);
    int b = dist(rng);
    int diff = a - b;
    return std::make_tuple(a, b, diff);
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        auto [a, b, diff] = generateAndDiff(10, 1000);
        std::cout << "Test " << i << ": A=" << a << ", B=" << b << ", Difference=" << diff << "\n";
    }
    return 0;
}