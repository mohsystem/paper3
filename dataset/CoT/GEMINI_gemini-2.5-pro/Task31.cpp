#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Given one side of a DNA strand, returns the other complementary side.
 * 'A' and 'T' are complements, 'C' and 'G' are complements.
 * 
 * @param dna The input DNA strand (e.g., "ATTGC").
 * @return The complementary DNA strand (e.g., "TAACG").
 */
std::string DNAStrand(const std::string& dna) {
    std::string complement = "";
    // Reserve memory to avoid multiple reallocations
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
                // Assuming valid input as per problem description
                break;
        }
    }
    return complement;
}

int main() {
    std::vector<std::string> testCases = {
        "ATTGC",
        "GTAT",
        "AAAA",
        "GATTACA",
        "CGCG"
    };
    
    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& test : testCases) {
        std::string result = DNAStrand(test);
        std::cout << "Input: " << test << " -> Output: " << result << std::endl;
    }
    
    return 0;
}