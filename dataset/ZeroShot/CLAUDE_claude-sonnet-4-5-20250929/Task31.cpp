
#include <iostream>
#include <string>
#include <stdexcept>

std::string getDNAComplement(const std::string& dna) {
    if (dna.empty()) {
        return "";
    }
    
    std::string complement;
    complement.reserve(dna.length());
    
    for (char nucleotide : dna) {
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
                throw std::invalid_argument(std::string("Invalid DNA nucleotide: ") + nucleotide);
        }
    }
    
    return complement;
}

int main() {
    // Test case 1
    std::cout << "Input: ATTGC" << std::endl;
    std::cout << "Output: " << getDNAComplement("ATTGC") << std::endl;
    std::cout << "Expected: TAACG\\n" << std::endl;
    
    // Test case 2
    std::cout << "Input: GTAT" << std::endl;
    std::cout << "Output: " << getDNAComplement("GTAT") << std::endl;
    std::cout << "Expected: CATA\\n" << std::endl;
    
    // Test case 3
    std::cout << "Input: AAAA" << std::endl;
    std::cout << "Output: " << getDNAComplement("AAAA") << std::endl;
    std::cout << "Expected: TTTT\\n" << std::endl;
    
    // Test case 4
    std::cout << "Input: CGCG" << std::endl;
    std::cout << "Output: " << getDNAComplement("CGCG") << std::endl;
    std::cout << "Expected: GCGC\\n" << std::endl;
    
    // Test case 5
    std::cout << "Input: ATCGATCG" << std::endl;
    std::cout << "Output: " << getDNAComplement("ATCGATCG") << std::endl;
    std::cout << "Expected: TAGCTAGC\\n" << std::endl;
    
    return 0;
}
