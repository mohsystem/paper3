#include <iostream>
#include <random>
#include <cstdint>
#include <limits>
#include <cmath>

struct Result {
    int first;
    int second;
    int difference;
};

static int rand_in_range(int min, int max) {
    if (min > max) std::swap(min, max);
    static thread_local std::mt19937_64 gen([]{
        std::random_device rd;
        std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd()};
        return std::mt19937_64(seq);
    }());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

Result generateAndDiff(int min, int max) {
    int a = rand_in_range(min, max);
    int b = rand_in_range(min, max);
    int diff = std::abs(a - b);
    return Result{a, b, diff};
}

int main() {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        Result r = generateAndDiff(10, 1000);
        std::cout << "Test " << (i + 1) << ": first=" << r.first
                  << ", second=" << r.second
                  << ", difference=" << r.difference << '\n';
    }
    return 0;
}