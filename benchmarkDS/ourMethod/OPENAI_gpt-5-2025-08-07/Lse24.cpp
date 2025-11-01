#include <iostream>
#include <random>
#include <chrono>
#include <string>

struct Result {
    int first;
    int second;
    int difference;
};

static std::mt19937& rng() {
    static std::random_device rd;
    static std::mt19937 gen(
        static_cast<std::mt19937::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count() ^
            static_cast<unsigned long long>(rd()) ^
            reinterpret_cast<std::uintptr_t>(&rd)
        )
    );
    return gen;
}

Result generateAndDiff() {
    std::uniform_int_distribution<int> dist(10, 1000); // inclusive
    int first = dist(rng());
    int second = dist(rng());
    int difference = first - second;
    return Result{first, second, difference};
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        Result r = generateAndDiff();
        std::cout << "Test " << i << ": First: " << r.first
                  << ", Second: " << r.second
                  << ", Difference (first - second): " << r.difference
                  << std::endl;
    }
    return 0;
}