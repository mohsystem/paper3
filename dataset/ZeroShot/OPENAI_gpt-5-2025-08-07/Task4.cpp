#include <iostream>
#include <string>
#include <vector>

bool isIsogram(const std::string& s) {
    if (s.empty()) return true;
    if (s.size() > 26) return false; // Pigeonhole principle for English alphabet

    unsigned int mask = 0U;
    for (unsigned char uc : s) {
        char c = static_cast<char>(uc);
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c + ('a' - 'A'));
        } else if (!(c >= 'a' && c <= 'z')) {
            return false; // invalid character (non-letter)
        }
        int idx = c - 'a';
        unsigned int bit = 1U << idx;
        if (mask & bit) return false;
        mask |= bit;
    }
    return true;
}

int main() {
    std::vector<std::string> tests {
        "Dermatoglyphics",
        "aba",
        "moOse",
        "",
        "isogram"
    };
    std::cout.setf(std::ios::boolalpha);
    for (const auto& t : tests) {
        std::cout << t << " -> " << isIsogram(t) << '\n';
    }
    return 0;
}