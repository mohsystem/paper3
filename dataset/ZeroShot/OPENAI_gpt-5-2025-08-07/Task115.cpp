#include <cstdint>
#include <iostream>
#include <limits>

struct FlagsBF {
    unsigned read : 1;
    unsigned write : 1;
    unsigned execute : 1;
    unsigned role : 2;      // 0-3
    unsigned priority : 4;  // 0-15
};

static bool validate(unsigned r, unsigned w, unsigned x, unsigned role, unsigned prio) {
    if ((r > 1U) || (w > 1U) || (x > 1U)) return false;
    if (role > 3U) return false;
    if (prio > 15U) return false;
    return true;
}

uint32_t build_packed(unsigned r, unsigned w, unsigned x, unsigned role, unsigned prio) {
    if (!validate(r, w, x, role, prio)) {
        return std::numeric_limits<uint32_t>::max();
    }
    uint32_t p = 0U;
    p |= (r & 1U) << 0;
    p |= (w & 1U) << 1;
    p |= (x & 1U) << 2;
    p |= (role & 3U) << 3;
    p |= (prio & 0xFU) << 5;
    return p;
}

FlagsBF unpack_to_struct(uint32_t packed) {
    FlagsBF f{};
    f.read = (packed >> 0) & 1U;
    f.write = (packed >> 1) & 1U;
    f.execute = (packed >> 2) & 1U;
    f.role = (packed >> 3) & 3U;
    f.priority = (packed >> 5) & 0xFU;
    return f;
}

int main() {
    unsigned tests[5][5] = {
        {1, 1, 0, 2, 7},
        {0, 0, 1, 1, 0},
        {1, 0, 1, 3, 15},
        {1, 1, 1, 4, 2},   // invalid role
        {1, 0, 0, 0, 16}   // invalid priority
    };
    for (int i = 0; i < 5; ++i) {
        uint32_t packed = build_packed(tests[i][0], tests[i][1], tests[i][2], tests[i][3], tests[i][4]);
        if (packed == std::numeric_limits<uint32_t>::max()) {
            std::cout << "Test " << (i + 1) << ": error=invalid input\n";
        } else {
            FlagsBF f = unpack_to_struct(packed);
            std::cout << "Test " << (i + 1) << ": packed=" << packed
                      << " unpacked={read=" << f.read
                      << ", write=" << f.write
                      << ", execute=" << f.execute
                      << ", role=" << f.role
                      << ", priority=" << f.priority << "}\n";
        }
    }
    return 0;
}