// Chain-of-Through Secure Development Steps:
// 1) Problem understanding: Convert to Jaden Case; return empty string for empty input.
// 2) Security requirements: Avoid undefined behavior with ctype by casting to unsigned char; preserve whitespace.
// 3) Secure coding generation: Character-wise processing; no dynamic memory hazards.
// 4) Code review: Safe handling of empty strings; preserves punctuation and whitespace.
// 5) Secure code output: Validated with 5 test cases.

#include <iostream>
#include <string>
#include <cctype>

std::string toJadenCase(const std::string& s) {
    if (s.empty()) return std::string();

    std::string out;
    out.reserve(s.size());
    bool startOfWord = true;

    for (unsigned char uc : s) {
        if (std::isspace(uc)) {
            out.push_back(static_cast<char>(uc));
            startOfWord = true;
        } else {
            if (startOfWord) {
                out.push_back(static_cast<char>(std::toupper(uc)));
            } else {
                out.push_back(static_cast<char>(std::tolower(uc)));
            }
            startOfWord = false;
        }
    }
    return out;
}

int main() {
    std::string tests[5] = {
        "How can mirrors be real if our eyes aren't real",
        "most trees are blue",
        "",
        "   ",
        "a mixed-CASE input, with punctuation! and\ttabs"
    };

    for (const auto& t : tests) {
        std::string res = toJadenCase(t);
        std::cout << res << std::endl;
    }
    return 0;
}