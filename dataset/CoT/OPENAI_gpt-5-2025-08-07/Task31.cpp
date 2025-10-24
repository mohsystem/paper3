// C++ implementation
// Chain-of-Through process (high-level):
// 1) Understand: compute complementary DNA.
// 2) Security: validate inputs; throw on invalid char.
// 3) Secure coding: no raw pointers for input; reserve capacity.
// 4) Review: use exceptions for error handling; no UB.
// 5) Output: function returns string; tests included.

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

std::string dnaComplement(const std::string& dna) {
    std::string out;
    out.reserve(dna.size());
    for (size_t i = 0; i < dna.size(); ++i) {
        char c = dna[i];
        switch (c) {
            case 'A': case 'a': out.push_back('T'); break;
            case 'T': case 't': out.push_back('A'); break;
            case 'C': case 'c': out.push_back('G'); break;
            case 'G': case 'g': out.push_back('C'); break;
            default:
                throw std::invalid_argument(std::string("Invalid DNA character at index ") + std::to_string(i) + ": " + c);
        }
    }
    return out;
}

// 5 test cases
int main() {
    std::vector<std::string> tests = {
        "ATTGC",
        "GTAT",
        "A",
        "GATTACA",
        "CCGGTTAA"
    };
    for (const auto& t : tests) {
        try {
            std::cout << t << " -> " << dnaComplement(t) << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Error for \"" << t << "\": " << ex.what() << "\n";
        }
    }
    return 0;
}