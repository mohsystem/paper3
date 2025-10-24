#include <iostream>
#include <vector>
#include <string>

std::string odd_or_even(const std::vector<long long>& nums) {
    unsigned int parity = 0U;
    for (long long x : nums) {
        parity ^= static_cast<unsigned long long>(x) & 1ULL;
    }
    return parity ? "odd" : "even";
}

int main() {
    // 5 test cases
    std::vector<std::vector<long long>> tests = {
        {},                 // empty -> even
        {0},                // even
        {0, 1, 4},          // odd
        {0, -1, -5},        // even
        {1, 3, 5, 7}        // even (sum 16)
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << ": " << odd_or_even(tests[i]) << std::endl;
    }

    return 0;
}