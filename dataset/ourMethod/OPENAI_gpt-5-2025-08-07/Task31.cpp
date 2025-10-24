#include <iostream>
#include <string>
#include <vector>

// Returns the complementary DNA strand for a given input.
// Rules: A <-> T, C <-> G
// Returns empty string on invalid input (non-ACGT) or empty input.
std::string dnaComplement(const std::string& dna) {
    if (dna.empty()) {
        return std::string();
    }
    std::string out;
    out.reserve(dna.size());
    for (char ch : dna) {
        switch (ch) {
            case 'A': out.push_back('T'); break;
            case 'T': out.push_back('A'); break;
            case 'C': out.push_back('G'); break;
            case 'G': out.push_back('C'); break;
            default: return std::string(); // invalid character
        }
    }
    return out;
}

int main() {
    struct TestCase {
        std::string input;
        std::string expected;
        bool expectValid;
    };

    std::vector<TestCase> tests = {
        {"ATTGC", "TAACG", true},
        {"GTAT",  "CATA",  true},
        {"AAAA",  "TTTT",  true},
        {"CGCG",  "GCGC",  true},
        {"ABCD",  "",      false}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        const auto& t = tests[i];
        std::string result = dnaComplement(t.input);
        bool valid = !result.empty() || t.input.empty(); // empty result means invalid for non-empty input
        std::cout << "Test " << (i + 1) << ": input=\"" << t.input << "\" -> ";
        if (!valid) {
            std::cout << "INVALID";
        } else {
            std::cout << "\"" << result << "\"";
        }
        if (t.expectValid) {
            std::cout << " | expected=\"" << t.expected << "\""
                      << " | " << ((result == t.expected) ? "PASS" : "FAIL");
        } else {
            std::cout << " | expected=INVALID"
                      << " | " << ((!valid) ? "PASS" : "FAIL");
        }
        std::cout << std::endl;
    }
    return 0;
}