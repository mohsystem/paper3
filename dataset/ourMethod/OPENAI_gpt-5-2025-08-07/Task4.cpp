#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

static inline bool isAsciiLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static inline char toLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<char>(c - 'A' + 'a');
    return c;
}

// Determines if the input string (letters only) is an isogram (case-insensitive).
// Returns true for empty string. Returns false if any non-letter is present.
bool isIsogram(const std::string& s) {
    uint32_t seen = 0u;
    for (char ch : s) {
        if (!isAsciiLetter(ch)) {
            return false; // input validation: only letters allowed
        }
        char lower = toLowerAscii(ch);
        uint32_t bit = 1u << static_cast<unsigned>(lower - 'a');
        if ((seen & bit) != 0u) {
            return false; // repeated letter
        }
        seen |= bit;
    }
    return true;
}

int main() {
    std::vector<std::string> tests = {
        "Dermatoglyphics", // true
        "aba",             // false
        "moOse",           // false
        "",                // true
        "isogram"          // true
    };

    std::cout << std::boolalpha;
    for (const auto& t : tests) {
        bool result = isIsogram(t);
        std::cout << "Input: \"" << t << "\" -> " << result << "\n";
    }

    return 0;
}