#include <iostream>
#include <string>
#include <vector>

std::string dnaStrand(std::string dna) {
    std::string result = "";
    result.reserve(dna.length()); // Pre-allocate memory for efficiency
    for (char c : dna) {
        switch (c) {
            case 'A':
                result += 'T';
                break;
            case 'T':
                result += 'A';
                break;
            case 'C':
                result += 'G';
                break;
            case 'G':
                result += 'C';
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

int main() {
    std::vector<std::string> testCases = {"ATTGC", "GTAT", "AAAA", "CGCG", "TAGCTAGC"};
    for (const auto& testCase : testCases) {
        std::cout << "Input: " << testCase << " --> Output: " << dnaStrand(testCase) << std::endl;
    }
    return 0;
}