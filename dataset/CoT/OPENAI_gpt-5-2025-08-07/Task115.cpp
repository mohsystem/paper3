// Chain-of-Through secure code generation process embedded as comments.
// 1) Problem understanding: Define a struct with bit fields to represent permissions.
//    Fields: read(1), write(1), execute(1), role(2), level(5).
// 2) Security requirements: Mask inputs to field widths to avoid UB, no reinterpret casts,
//    use fixed-width types and safe shifts.
// 3) Secure coding generation: Provide creation, pack, and unpack functions.
// 4) Code review: All bit ops are masked; unpack/pack are pure; no external input parsing.
// 5) Secure code output: Final code with 5 test cases.

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

struct Permissions {
    unsigned int read   : 1;
    unsigned int write  : 1;
    unsigned int execute: 1;
    unsigned int role   : 2;
    unsigned int level  : 5;
};

// Create a Permissions struct from values, masking to safe ranges.
Permissions make_permissions(unsigned int read, unsigned int write, unsigned int execute,
                             unsigned int role, unsigned int level) {
    Permissions p{};
    p.read    = read   & 0x1;
    p.write   = write  & 0x1;
    p.execute = execute& 0x1;
    p.role    = role   & 0x3;
    p.level   = level  & 0x1F;
    return p;
}

// Pack bit-field struct into 16-bit unsigned integer without relying on layout.
std::uint16_t pack_permissions(const Permissions& p) {
    std::uint16_t v = 0;
    v |= (static_cast<std::uint16_t>(p.read)    & 0x1)  << 0;
    v |= (static_cast<std::uint16_t>(p.write)   & 0x1)  << 1;
    v |= (static_cast<std::uint16_t>(p.execute) & 0x1)  << 2;
    v |= (static_cast<std::uint16_t>(p.role)    & 0x3)  << 3;
    v |= (static_cast<std::uint16_t>(p.level)   & 0x1F) << 5;
    return v;
}

// Unpack a 16-bit integer into the bit-field struct.
Permissions unpack_permissions(std::uint16_t packed) {
    return make_permissions(
        (packed >> 0) & 0x1,
        (packed >> 1) & 0x1,
        (packed >> 2) & 0x1,
        (packed >> 3) & 0x3,
        (packed >> 5) & 0x1F
    );
}

std::string to_string(const Permissions& p, std::uint16_t packed) {
    char buf[128];
    std::snprintf(buf, sizeof(buf),
                  "Permissions(read=%u, write=%u, execute=%u, role=%u, level=%u, packed=0x%04X)",
                  p.read, p.write, p.execute, p.role, p.level, static_cast<unsigned>(packed));
    return std::string(buf);
}

int main() {
    unsigned tests[5][5] = {
        {1,0,1,2,17},
        {0,1,1,1,31},
        {1,1,1,3,0},
        {0,0,0,0,5},
        {1,1,0,2,29}
    };

    for (int i = 0; i < 5; ++i) {
        Permissions p = make_permissions(tests[i][0], tests[i][1], tests[i][2], tests[i][3], tests[i][4]);
        std::uint16_t packed = pack_permissions(p);
        std::cout << "Test " << (i+1) << " -> " << to_string(p, packed) << "\n";
        Permissions u = unpack_permissions(packed);
        std::uint16_t repacked = pack_permissions(u);
        std::cout << "Unpacked " << (i+1) << " -> " << to_string(u, repacked) << "\n";
    }
    return 0;
}