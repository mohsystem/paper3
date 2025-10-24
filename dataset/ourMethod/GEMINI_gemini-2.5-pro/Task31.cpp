#include <iostream>
#include <string>
#include <vector>

// Function to find the complementary DNA strand
std::string dnaStrand(const std::string& dna) {
    std::string result = "";
    // Reserve capacity to avoid multiple reallocations for performance
    if (!dna.empty()) {
        result.reserve(dna.length());
    }
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
            // According to the prompt, the input will only contain 'A', 'T', 'C', 'G'.
            // A more robust implementation might handle other characters.
        }
    }
    return result;
}

// Main function with test cases
int main() {
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"ATTGC", "TAACG"},
        {"GTAT", "CATA"},
        {"AAAA", "TTTT"},
        {"CGCG", "GCGC"},
        {"GATTACA", "CTAATGT"}
    };

    int test_num = 1;
    for (const auto& test_case : test_cases) {
        const std::string& input = test_case.first;
        const std::string& expected = test_case.second;
        std::string actual = dnaStrand(input);

        std::cout << "Test Case " << test_num++ << ":\n";
        std::cout << "Input:    \"" << input << "\"\n";
        std::cout << "Expected: \"" << expected << "\"\n";
        std::cout << "Actual:   \"" << actual << "\"\n";
        if (actual == expected) {
            std::cout << "Result:   PASS\n";
        } else {
            std::cout << "Result:   FAIL\n";
        }
        std::cout << "---------------------\n";
    }

    return 0;
}