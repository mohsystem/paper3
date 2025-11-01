#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>
#include <cstdint>

uint64_t random_u64() {
    std::random_device rd;
    uint64_t a = static_cast<uint64_t>(rd());
    uint64_t b = static_cast<uint64_t>(rd());
    return (a << 32) ^ b;
}

uint64_t generateRandomGreaterThan(uint64_t minExclusive) {
    if (minExclusive >= std::numeric_limits<uint64_t>::max()) {
        throw std::invalid_argument("minExclusive too large");
    }
    uint64_t x;
    do {
        x = random_u64();
    } while (x <= minExclusive);
    return x;
}

int main() {
    for (int i = 0; i < 5; ++i) {
        std::cout << generateRandomGreaterThan(1000) << '\n';
    }
    return 0;
}