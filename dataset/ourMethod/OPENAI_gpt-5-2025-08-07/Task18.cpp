#include <iostream>
#include <vector>
#include <cstdint>
#include <limits>
#include <string>

struct BinToIntResult {
    bool ok;
    uint64_t value;
    const char* error; // points to a constant string literal
};

// Converts a big-endian bit array (MSB first) to an unsigned 64-bit integer.
// Validates that each element is 0 or 1 and that the result does not overflow uint64_t.
// Empty input is treated as 0.
BinToIntResult binaryArrayToInt(const std::vector<int>& bits) {
    BinToIntResult res{true, 0u, nullptr};

    uint64_t acc = 0;
    for (size_t i = 0; i < bits.size(); ++i) {
        int b = bits[i];
        if (b != 0 && b != 1) {
            res.ok = false;
            res.error = "Invalid bit value (must be 0 or 1)";
            return res;
        }
        // Check for overflow of acc*2 + b
        // Condition: acc > (UINT64_MAX - b) / 2
        if (acc > (std::numeric_limits<uint64_t>::max() - static_cast<uint64_t>(b)) / 2u) {
            res.ok = false;
            res.error = "Overflow: value exceeds uint64_t range";
            return res;
        }
        acc = (acc << 1) | static_cast<uint64_t>(b);
    }
    res.value = acc;
    return res;
}

static void printArray(const std::vector<int>& bits) {
    std::cout << "[";
    for (size_t i = 0; i < bits.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << bits[i];
    }
    std::cout << "]";
}

int main() {
    std::vector<std::vector<int>> tests = {
        {0, 0, 0, 1},
        {0, 0, 1, 0},
        {0, 1, 0, 1},
        {1, 0, 0, 1},
        {1, 1, 1, 1}
    };

    for (const auto& t : tests) {
        BinToIntResult r = binaryArrayToInt(t);
        std::cout << "Testing: ";
        printArray(t);
        if (r.ok) {
            std::cout << " ==> " << r.value << "\n";
        } else {
            std::cout << " ==> error: " << r.error << "\n";
        }
    }

    return 0;
}