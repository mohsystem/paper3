#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdint>

// Detects if the input string is a pangram (contains every letter A-Z at least once, case-insensitive).
// Parameters: s - input string
// Returns: true if pangram, false otherwise
bool isPangram(const std::string &s) {
    const uint32_t allMask = (1u << 26) - 1u; // 26 ones (0x03FFFFFF)
    uint32_t mask = 0u;

    for (char ch : s) {
        unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalpha(uch)) {
            char lower = static_cast<char>(std::tolower(uch));
            int idx = static_cast<int>(lower) - static_cast<int>('a');
            if (idx >= 0 && idx < 26) {
                mask |= (1u << idx);
                if (mask == allMask) {
                    return true; // Early exit if all letters found
                }
            }
        }
    }
    return mask == allMask;
}

int main() {
    std::vector<std::string> tests = {
        "The quick brown fox jumps over the lazy dog",
        "Sphinx of black quartz, judge my vow!",
        "Hello, World!",
        "",
        "Pack my box with five dozen liquor jugs 12345!!!"
    };

    for (const auto &t : tests) {
        bool result = isPangram(t);
        std::cout << (result ? "True" : "False") << std::endl;
    }

    return 0;
}