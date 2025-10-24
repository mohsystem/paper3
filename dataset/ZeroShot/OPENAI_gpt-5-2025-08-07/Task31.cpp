#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

std::string dna_complement(const std::string& dna) {
    if (dna.empty()) {
        throw std::invalid_argument("Input DNA string cannot be empty.");
    }
    std::string out;
    out.reserve(dna.size());
    for (size_t i = 0; i < dna.size(); ++i) {
        unsigned char uc = static_cast<unsigned char>(dna[i]);
        char up = static_cast<char>(std::toupper(uc));
        switch (up) {
            case 'A': out.push_back('T'); break;
            case 'T': out.push_back('A'); break;
            case 'C': out.push_back('G'); break;
            case 'G': out.push_back('C'); break;
            default:
                throw std::invalid_argument(std::string("Invalid DNA character at index ") + std::to_string(i) + ": '" + dna[i] + "'");
        }
    }
    return out;
}

int main() {
    const std::string tests[] = {
        "ATTGC",   // TAACG
        "GTAT",    // CATA
        "A",       // T
        "aaaa",    // TTTT
        "gCatTa"   // CGTAAT
    };
    for (const auto& t : tests) {
        try {
            std::string res = dna_complement(t);
            std::cout << "Input: " << t << " -> " << res << "\n";
        } catch (const std::exception& e) {
            std::cout << "Input: " << t << " -> Error: " << e.what() << "\n";
        }
    }
    return 0;
}