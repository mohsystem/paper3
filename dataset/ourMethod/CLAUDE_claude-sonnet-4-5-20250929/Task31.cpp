
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstddef>

// Function to return the complementary DNA strand
// Security: Input validation ensures only valid DNA characters (A, T, C, G)
// Returns empty string for empty input as per problem statement
std::string getDNAComplement(const std::string& dna) {
    // Validate input is not null/empty - problem states DNA is never empty
    // but we validate for safety
    if (dna.empty()) {
        return "";
    }
    
    // Validate length to prevent excessive memory allocation
    // Maximum reasonable DNA length check (preventing DoS via memory exhaustion)
    const size_t MAX_DNA_LENGTH = 10000000; // 10 million characters
    if (dna.length() > MAX_DNA_LENGTH) {
        throw std::invalid_argument("DNA string exceeds maximum allowed length");
    }
    
    // Pre-allocate result string with exact size needed (no reallocation)
    std::string complement;
    complement.reserve(dna.length());
    
    // Process each character with validation
    for (size_t i = 0; i < dna.length(); ++i) {
        char nucleotide = dna[i];
        
        // Input validation: only accept valid DNA characters
        // This prevents injection of unexpected characters
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
                // Invalid character detected - fail closed with clear error
                throw std::invalid_argument("Invalid DNA character encountered");
        }
    }
    
    return complement;
}

// Test cases
int main() {
    try {
        // Test case 1: Standard example from problem
        std::string result1 = getDNAComplement("ATTGC");
        std::cout << "Test 1: ATTGC -> " << result1 << " (Expected: TAACG)" << std::endl;
        
        // Test case 2: Standard example from problem
        std::string result2 = getDNAComplement("GTAT");
        std::cout << "Test 2: GTAT -> " << result2 << " (Expected: CATA)" << std::endl;
        
        // Test case 3: All A's
        std::string result3 = getDNAComplement("AAAA");
        std::cout << "Test 3: AAAA -> " << result3 << " (Expected: TTTT)" << std::endl;
        
        // Test case 4: All combinations
        std::string result4 = getDNAComplement("ATCG");
        std::cout << "Test 4: ATCG -> " << result4 << " (Expected: TAGC)" << std::endl;
        
        // Test case 5: Longer sequence
        std::string result5 = getDNAComplement("GCGCGCGC");
        std::cout << "Test 5: GCGCGCGC -> " << result5 << " (Expected: CGCGCGCG)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
