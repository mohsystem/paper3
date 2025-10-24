#include <iostream>
#include <string>

bool isPangram(const std::string& s) {
    if (s.empty()) return false;
    unsigned int mask = 0;
    const unsigned int FULL = (1u << 26) - 1u;
    for (unsigned char ch : s) {
        if (ch >= 'A' && ch <= 'Z') ch = static_cast<unsigned char>(ch + 32);
        if (ch >= 'a' && ch <= 'z') {
            mask |= (1u << (ch - 'a'));
            if (mask == FULL) break;
        }
    }
    return mask == FULL;
}

int main() {
    std::string tests[5] = {
        "The quick brown fox jumps over the lazy dog",
        "Hello, World!",
        "",
        "Pack my box with five dozen liquor jugs.",
        "The quick brown fox jumps over the lay dog"
    };
    for (const auto& t : tests) {
        std::cout << (isPangram(t) ? "True" : "False") << std::endl;
    }
    return 0;
}