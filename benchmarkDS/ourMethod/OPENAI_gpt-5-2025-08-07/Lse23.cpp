#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <algorithm>

static constexpr int DEFAULT_MIN = 1001;
static constexpr int DEFAULT_MAX = 1'000'000; // inclusive

// Thread-local RNG seeded from random_device
static std::string generate_output(int min_value) {
    static thread_local std::mt19937_64 rng([]() {
        std::random_device rd;
        std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
        return std::mt19937_64(seq);
    }());

    int low = std::max(min_value, DEFAULT_MIN);
    int high = DEFAULT_MAX;
    if (low > high) {
        low = DEFAULT_MIN; // fail closed
    }

    std::uniform_int_distribution<int> dist(low, high);
    int a = dist(rng);
    int b = dist(rng);
    int c = dist(rng);

    long long sum = static_cast<long long>(a) + b + c;
    long long sum2 = static_cast<long long>(a) + b + c;

    std::ostringstream oss;
    oss << "a : " << a << " b : " << b << " c : " << c
        << " sum: " << sum << " sum2: " << sum2;
    return oss.str();
}

int main() {
    // 5 test cases
    std::cout << generate_output(1001) << '\n';
    std::cout << generate_output(1001) << '\n';
    std::cout << generate_output(1001) << '\n';
    std::cout << generate_output(1001) << '\n';
    std::cout << generate_output(1001) << '\n';
    return 0;
}