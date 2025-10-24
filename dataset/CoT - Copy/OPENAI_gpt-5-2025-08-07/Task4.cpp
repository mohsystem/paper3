// Chain-of-Through process:
// 1) Problem understanding: Determine if a letters-only string is an isogram (no repeating letters), case-insensitive; empty is true.
// 2) Security requirements: Validate input characters, avoid undefined behavior, handle only ASCII letters.
// 3) Secure coding generation: Manual ASCII lowercase conversion, track seen letters with fixed-size array.
// 4) Code review: No external inputs, no dynamic allocations, clear bounds checks.
// 5) Secure code output: Final code adheres to constraints and avoids vulnerabilities.

#include <iostream>
#include <string>
#include <array>

bool isIsogram(const std::string& s) {
    if (s.empty()) return true;
    std::array<bool, 26> seen{};
    for (unsigned char uc : s) {
        char c;
        if (uc >= 'A' && uc <= 'Z') {
            c = static_cast<char>(uc - 'A' + 'a');
        } else if (uc >= 'a' && uc <= 'z') {
            c = static_cast<char>(uc);
        } else {
            // Non-letter violates the letters-only assumption
            return false;
        }
        int idx = c - 'a';
        if (seen[idx]) return false;
        seen[idx] = true;
    }
    return true;
}

static void runTest(const std::string& input) {
    bool result = isIsogram(input);
    std::cout << "\"" << input << "\" --> " << (result ? "true" : "false") << "\n";
}

int main() {
    runTest("Dermatoglyphics"); // true
    runTest("aba");             // false
    runTest("moOse");           // false
    runTest("");                // true
    runTest("isogram");         // true
    return 0;
}