// Chain-of-Through Step 1: Problem understanding
// Purpose: Concatenate multiple strings from a vector input.
// Inputs: std::vector<std::string> possibly containing empty strings.
// Output: Single concatenated std::string.

// Chain-of-Through Step 2: Security requirements
// - Handle empty vector gracefully.
// - Reserve capacity to avoid repeated reallocations.

// Chain-of-Through Step 3: Secure coding generation
#include <iostream>
#include <string>
#include <vector>
#include <limits>

std::string concatenateStrings(const std::vector<std::string>& parts) {
    if (parts.empty()) return std::string();

    // Compute total size with overflow guard
    size_t total = 0;
    for (const auto& s : parts) {
        if (s.size() > std::numeric_limits<size_t>::max() - total) {
            // Overflow guard: fall back to no reserve
            total = 0;
            break;
        }
        total += s.size();
    }

    std::string out;
    if (total > 0) out.reserve(total);
    for (const auto& s : parts) {
        out.append(s);
    }
    return out;
}

// Chain-of-Through Step 4: Code review (comments)
// - Overflow checked during size computation.
// - Reserve used when possible.
// - Safe appends; no raw pointers.

// Chain-of-Through Step 5: Secure code output (final)
int main() {
    // 5 test cases
    std::vector<std::vector<std::string>> tests = {
        {"Hello", " ", "World", "!"},
        {"", "Secure", "", "Concat"},
        {},
        {"", "", "non-empty", "", "end"},
        {"Unicode: ", "😀", " ", "文字", " ", "✓"}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string result = concatenateStrings(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << result << "\n";
    }
    return 0;
}