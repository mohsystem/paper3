
#include <iostream>
#include <string>
#include <stdexcept>

std::string makeComplement(const std::string& dna) {
    std::string complement;
    complement.reserve(dna.length());
    
    for (size_t i = 0; i < dna.length(); i++) {
        char nucleotide = dna[i];
        switch (nucleotide) {
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
                throw std::invalid_argument("Invalid DNA nucleotide: " + std::string(1, nucleotide));
        }
    }
    return complement;
}

int main() {
    // Test case 1
    std::cout << "Input: ATTGC, Output: " << makeComplement("ATTGC") << std::endl;
    
    // Test case 2
    std::cout << "Input: GTAT, Output: " << makeComplement("GTAT") << std::endl;
    
    // Test case 3
    std::cout << "Input: AAAA, Output: " << makeComplement("AAAA") << std::endl;
    
    // Test case 4
    std::cout << "Input: CGCG, Output: " << makeComplement("CGCG") << std::endl;
    
    // Test case 5
    std::cout << "Input: ATCGATCG, Output: " << makeComplement("ATCGATCG") << std::endl;
    
    return 0;
}
