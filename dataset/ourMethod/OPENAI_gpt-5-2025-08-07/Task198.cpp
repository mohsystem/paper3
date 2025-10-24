#include <iostream>
#include <vector>
#include <cstdint>

// Park-Miller LCG parameters
static const uint64_t MOD = 2147483647ULL; // 2^31 - 1
static const uint64_t MUL = 48271ULL;
static uint64_t seed = 123456789ULL;

static uint32_t rngNext() {
    seed = (seed * MUL) % MOD; // 1..MOD-1
    return static_cast<uint32_t>(seed);
}

static int rngUniform(int k) {
    if (k <= 0) throw std::invalid_argument("k must be positive");
    uint64_t m1 = MOD - 1ULL; // 2147483646
    uint64_t limit = (m1 / static_cast<uint64_t>(k)) * static_cast<uint64_t>(k);
    while (true) {
        uint64_t r = static_cast<uint64_t>(rngNext()) - 1ULL; // 0..m1-1
        if (r < limit) {
            return static_cast<int>(r % static_cast<uint64_t>(k));
        }
    }
}

// Given API
int rand7() {
    return rngUniform(7) + 1; // 1..7
}

// Required function
int rand10() {
    while (true) {
        int a = rand7();
        int b = rand7();
        int num = (a - 1) * 7 + b; // 1..49
        if (num <= 40) {
            return 1 + (num - 1) % 10;
        }
    }
}

std::vector<int> runRand10NTimes(int n) {
    if (n < 1 || n > 100000) throw std::invalid_argument("n out of range");
    std::vector<int> res;
    res.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) res.push_back(rand10());
    return res;
}

static void printArray(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << arr[i];
    }
    std::cout << "]\n";
}

int main() {
    try {
        int tests[5] = {1, 2, 3, 5, 10};
        for (int n : tests) {
            auto out = runRand10NTimes(n);
            printArray(out);
        }
    } catch (const std::exception& e) {
        // Fail closed; do not leak sensitive info
        std::cout << "[]\n";
    }
    return 0;
}