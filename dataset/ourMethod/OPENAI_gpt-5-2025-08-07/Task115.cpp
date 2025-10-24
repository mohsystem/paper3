#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// Define a struct with bit fields representing a 16-bit status register:
// [bit0=isActive][bit1=isError][bits2-4=priority(3)][bits5-8=level(4)][bits9-15=code(7)]
struct StatusBits {
    unsigned isActive : 1;
    unsigned isError  : 1;
    unsigned priority : 3;
    unsigned level    : 4;
    unsigned code     : 7;
};

struct PackResult {
    bool ok;
    uint16_t value;
};

struct UnpackResult {
    bool ok;
    StatusBits bits;
    uint16_t raw;
};

static constexpr uint16_t ACTIVE_SHIFT   = 0;
static constexpr uint16_t ERROR_SHIFT    = 1;
static constexpr uint16_t PRIORITY_SHIFT = 2;
static constexpr uint16_t LEVEL_SHIFT    = 5;
static constexpr uint16_t CODE_SHIFT     = 9;

static constexpr uint16_t ACTIVE_MASK   = static_cast<uint16_t>(0x1u << ACTIVE_SHIFT);
static constexpr uint16_t ERROR_MASK    = static_cast<uint16_t>(0x1u << ERROR_SHIFT);
static constexpr uint16_t PRIORITY_MASK = static_cast<uint16_t>(0x7u << PRIORITY_SHIFT);
static constexpr uint16_t LEVEL_MASK    = static_cast<uint16_t>(0xFu << LEVEL_SHIFT);
static constexpr uint16_t CODE_MASK     = static_cast<uint16_t>(0x7Fu << CODE_SHIFT);

static constexpr unsigned MAX_PRIORITY = 7u;
static constexpr unsigned MAX_LEVEL    = 15u;
static constexpr unsigned MAX_CODE     = 127u;

static bool inRange(unsigned v, unsigned minv, unsigned maxv) {
    return v >= minv && v <= maxv;
}

PackResult packStatus(unsigned isActive, unsigned isError, unsigned priority, unsigned level, unsigned code) {
    PackResult r{};
    if (!inRange(isActive, 0u, 1u) || !inRange(isError, 0u, 1u) ||
        !inRange(priority, 0u, MAX_PRIORITY) ||
        !inRange(level, 0u, MAX_LEVEL) ||
        !inRange(code, 0u, MAX_CODE)) {
        r.ok = false;
        r.value = 0;
        return r;
    }
    uint16_t v = 0;
    v |= static_cast<uint16_t>((isActive & 0x1u) << ACTIVE_SHIFT);
    v |= static_cast<uint16_t>((isError  & 0x1u) << ERROR_SHIFT);
    v |= static_cast<uint16_t>((priority & 0x7u) << PRIORITY_SHIFT);
    v |= static_cast<uint16_t>((level    & 0xFu) << LEVEL_SHIFT);
    v |= static_cast<uint16_t>((code     & 0x7Fu) << CODE_SHIFT);
    r.ok = true;
    r.value = v;
    return r;
}

UnpackResult unpackStatus(uint16_t packed) {
    UnpackResult ur{};
    ur.ok = true;
    ur.raw = packed;
    StatusBits b{};
    b.isActive = (packed & ACTIVE_MASK) >> ACTIVE_SHIFT;
    b.isError  = (packed & ERROR_MASK)  >> ERROR_SHIFT;
    b.priority = (packed & PRIORITY_MASK) >> PRIORITY_SHIFT;
    b.level    = (packed & LEVEL_MASK)    >> LEVEL_SHIFT;
    b.code     = (packed & CODE_MASK)     >> CODE_SHIFT;
    ur.bits = b;
    return ur;
}

static std::string toString(const StatusBits& b) {
    return std::string("StatusBits{isActive=") + (b.isActive ? "1" : "0") +
           ", isError=" + (b.isError ? "1" : "0") +
           ", priority=" + std::to_string(b.priority) +
           ", level=" + std::to_string(b.level) +
           ", code=" + std::to_string(b.code) + "}";
}

int main() {
    std::vector<std::vector<unsigned>> tests = {
        {1u, 0u, 3u, 8u, 42u},
        {0u, 1u, 7u, 15u, 127u},
        {1u, 1u, 0u, 0u, 0u},
        {0u, 0u, 5u, 9u, 100u},
        {1u, 0u, 9u, 2u, 5u} // invalid priority
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        const auto& t = tests[i];
        std::cout << "Test " << (i + 1) << " input: ["
                  << t[0] << ", " << t[1] << ", " << t[2] << ", " << t[3] << ", " << t[4] << "]\n";
        PackResult pr = packStatus(t[0], t[1], t[2], t[3], t[4]);
        if (!pr.ok) {
            std::cout << "Error: invalid parameters\n";
            continue;
        }
        std::cout << "Packed: " << pr.value << "\n";
        UnpackResult ur = unpackStatus(pr.value);
        std::cout << "Unpacked: " << toString(ur.bits) << "\n";
    }
    return 0;
}