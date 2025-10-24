#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * @brief Transforms a string based on the specified accumulation rule.
 * Example: accum("abcd") -> "A-Bb-Ccc-Dddd"
 * @param s The input string, containing only letters from a..z and A..Z.
 * @return The transformed string.
 */
std::string accum(const std::string& s) {
    std::string result;
    // Pre-allocating memory can be a good optimization to avoid reallocations.
    if (!s.empty()) {
        size_t n = s.length();
        // Exact size: n*(n+1)/2 for letters + (n-1) for hyphens
        size_t required_size = (n * (n + 1)) / 2 + n - 1;
        result.reserve(required_size);
    }

    for (size_t i = 0; i < s.length(); ++i) {
        if (i > 0) {
            result += '-';
        }
        result += static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
        for (size_t j = 0; j < i; ++j) {
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
        }
    }
    return result;
}

int main() {
    std::vector<std::string> testCases = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "Z",
        ""
    };

    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\" -> Output: \"" << accum(testCase) << "\"" << std::endl;
    }

    return 0;
}