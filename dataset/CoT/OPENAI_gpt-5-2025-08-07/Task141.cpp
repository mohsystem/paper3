// Chain-of-Through process via comments for secure coding.
// 1) Problem: reverse a given string by bytes (ASCII-safe).
// 2) Security: no raw pointers, no UB; handle large inputs via std::string.
// 3) Secure generation: use iterators; no exceptions required.
// 4) Review: no leaks, no concurrency issues.
// 5) Final secure output below.

#include <iostream>
#include <string>
#include <vector>

std::string reverseString(const std::string& input) {
    return std::string(input.rbegin(), input.rend());
}

int main() {
    std::vector<std::string> tests = {"", "hello", "A", "racecar", "Hello, World!"};
    for (const auto& t : tests) {
        std::string r = reverseString(t);
        std::cout << "Input: \"" << t << "\" -> Reversed: \"" << r << "\"\n";
    }
    return 0;
}