#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// Chain-of-Through:
// 1) Problem: Safely write input into a fixed-size buffer.
// 2) Security: Prevent overflow, handle null-equivalent, size validation.
// 3) Implementation: Return string limited to capacity.
// 4) Review: Avoid raw pointer overflow; use std::string safe operations.
// 5) Output: Final function.

std::string handle_input_to_fixed_buffer(const std::string& input, std::size_t buffer_size) {
    // Treat buffer_size as capacity (storable characters).
    return input.substr(0, buffer_size);
}

int main() {
    std::pair<std::string, std::size_t> tests[5] = {
        {"Hello", 10},
        {"HelloWorld", 5},
        {"", 8},
        {"SafeBuffer123", 12},
        {"EdgeCase", 0}
    };

    for (int i = 0; i < 5; ++i) {
        const auto& in = tests[i].first;
        std::size_t sz = tests[i].second;
        std::string out = handle_input_to_fixed_buffer(in, sz);
        std::cout << "Test " << (i + 1) << ": input=\"" << in << "\", size=" << sz << " -> \"" << out << "\"\n";
    }
    return 0;
}