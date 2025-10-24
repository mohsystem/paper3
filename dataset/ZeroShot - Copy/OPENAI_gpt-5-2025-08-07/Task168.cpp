#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

std::string tweakLetters(const std::string& s, const std::vector<int>& shifts) {
    if (s.size() != shifts.size()) {
        throw std::invalid_argument("String length and shifts length must match.");
    }
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char ch = s[i];
        if (ch >= 'a' && ch <= 'z') {
            int base = ch - 'a';
            int adj = shifts[i] % 26;
            if (adj < 0) adj += 26;
            int newIdx = (base + adj) % 26;
            out.push_back(static_cast<char>('a' + newIdx));
        } else {
            out.push_back(ch);
        }
    }
    return out;
}

int main() {
    try {
        std::cout << tweakLetters("apple", {0, 1, -1, 0, -1}) << "\n"; // aqold
        std::cout << tweakLetters("many", {0, 0, 0, -1}) << "\n";      // manx
        std::cout << tweakLetters("rhino", {1, 1, 1, 1, 1}) << "\n";   // sijop
        std::cout << tweakLetters("zzz", {1, 1, 1}) << "\n";           // aaa
        std::cout << tweakLetters("abc", {27, -28, 52}) << "\n";       // bzc
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}