#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Finds the complementary strand of a DNA sequence.
 * 'A' is complement of 'T', 'C' is complement of 'G'.
 * 
 * @param dna A string representing one side of the DNA.
 * @return The complementary DNA strand.
 */
std::string dnaStrand(const std::string& dna) {
    std::string complement = "";
    // Reserving memory upfront can be a performance optimization
    // to avoid multiple reallocations for long strings.
    complement.reserve(dna.length());
    
    for (char c : dna) {
        switch (c) {
            case 'A':
                complement += 'T';
                break;
            case 'T':
                complement += 'A';
                break;
            case 'C':
                complement += 'G';
                break;
            case 'G':
                complement += 'C';
                break;
            default:
                // As per prompt, input is always valid. A more robust solution
                // might handle or log invalid characters.
                complement += c;
                break;
        }
    }
    return complement;
}

int main() {
    std::vector<std::string> testCases = {"ATTGC", "GTAT", "AAAA", "CGCG", "TAACG"};

    for (const auto& testCase : testCases) {
        std::string result = dnaStrand(testCase);
        std::cout << "\"" << testCase << "\" --> \"" << result << "\"" << std::endl;
    }

    return 0;
}