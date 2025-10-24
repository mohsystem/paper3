// Chain-of-Through process:
// 1) Problem: Determine if s ends with ending.
// 2) Security: Use safe std::string operations, no raw pointers.
// 3) Secure coding: Bounds check before comparison.
// 4) Review: Ensure correct for empty ending and equal lengths.
// 5) Output: Final code with tests.
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

bool solution(const std::string& s, const std::string& ending) {
    if (ending.size() > s.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), s.rbegin());
}

int main() {
    std::vector<std::pair<std::string, std::string>> tests = {
        {"abc", "bc"},
        {"abc", "d"},
        {"hello", ""},
        {"", ""},
        {"abc", "abc"}
    };
    std::cout << std::boolalpha;
    for (const auto& p : tests) {
        std::cout << solution(p.first, p.second) << "\n";
    }
    return 0;
}