// Chain-of-Through Step 1: Problem understanding - detect pangram ignoring non-letters, case-insensitive.
// Chain-of-Through Step 2: Security requirements - safe character handling, avoid UB with tolower by casting to unsigned char.
// Chain-of-Through Step 3: Secure coding generation - check only 'a'..'z', early exit.
// Chain-of-Through Step 4: Code review - ensure no out-of-bounds and no reliance on locale specifics.
// Chain-of-Through Step 5: Secure code output - final safe implementation.

#include <iostream>
#include <string>
#include <cctype>
#include <array>
#include <vector>

bool isPangram(const std::string& s) {
    std::array<bool, 26> seen{};
    int count = 0;
    for (unsigned char uc : s) {
        char c = static_cast<char>(std::tolower(uc));
        if (c >= 'a' && c <= 'z') {
            int idx = c - 'a';
            if (!seen[idx]) {
                seen[idx] = true;
                ++count;
                if (count == 26) return true;
            }
        }
    }
    return count == 26;
}

int main() {
    std::vector<std::string> tests = {
        "The quick brown fox jumps over the lazy dog",
        "Hello, world!",
        "",
        "Sphinx of black quartz, judge my vow 123!!!",
        "abcdefghijklmno pqrstuvwxy"
    };
    for (const auto& t : tests) {
        bool res = isPangram(t);
        std::cout << (res ? "True" : "False") << "\n";
    }
    return 0;
}