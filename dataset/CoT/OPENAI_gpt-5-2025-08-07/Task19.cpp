// Chain-of-Through process in code generation:
// 1) Understand: Reverse words with length >= 5, preserve spaces.
// 2) Security: Avoid unsafe casts and unchecked access.
// 3) Implementation: Manual scan to preserve exact spacing.
// 4) Review: Bounds-checked loops, no UB.
// 5) Output: Include 5 test cases.

#include <iostream>
#include <string>

std::string spinWords(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        if (s[i] == ' ') {
            out.push_back(' ');
            ++i;
            continue;
        }
        size_t start = i;
        while (i < n && s[i] != ' ') ++i;
        size_t len = i - start;
        if (len >= 5) {
            for (size_t j = 0; j < len; ++j) {
                out.push_back(s[i - 1 - j]);
            }
        } else {
            out.append(s, start, len);
        }
    }
    return out;
}

int main() {
    const std::string tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "abcd efghi jklmn op qrstu vwxyz"
    };
    for (const auto& t : tests) {
        std::cout << spinWords(t) << "\n";
    }
    return 0;
}