#include <iostream>
#include <random>
#include <cstdint>
#include <limits>

static constexpr uint64_t MAX_SPAN = 1000000000ULL;

// Generates a random number strictly greater than 'min_val'.
// Uses std::random_device to seed a 64-bit Mersenne Twister and caps the span to avoid overflow.
uint64_t generateRandomAbove(uint64_t min_val) {
    if (min_val == std::numeric_limits<uint64_t>::max()) {
        throw std::invalid_argument("min too large");
    }
    const uint64_t lower = min_val + 1; // safe since min_val != max
    const uint64_t remaining = std::numeric_limits<uint64_t>::max() - lower; // >= 0
    const uint64_t span = (remaining < MAX_SPAN) ? remaining : MAX_SPAN; // span in [0, MAX_SPAN]
    const uint64_t upper = lower + span; // >= lower

    std::random_device rd; // typically non-deterministic
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(lower, upper);
    return dist(gen);
}

int main() {
    uint64_t tests[5] = {
        1000ULL,
        5000ULL,
        12345ULL,
        999999ULL,
        std::numeric_limits<uint64_t>::max() - 2ULL
    };

    for (uint64_t t : tests) {
        try {
            uint64_t v = generateRandomAbove(t);
            std::cout << "min=" << t << " -> random=" << v << "\n";
            if (!(v > t)) {
                throw std::runtime_error("Generated value not greater than min");
            }
        } catch (const std::exception& e) {
            std::cerr << "error\n";
            return 1;
        }
    }
    return 0;
}