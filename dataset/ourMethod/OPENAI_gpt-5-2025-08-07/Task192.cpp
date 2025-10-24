#include <iostream>
#include <vector>
#include <stdexcept>

static void validateInput(const std::vector<int>& nums) {
    if (nums.empty() || nums.size() > 1000) {
        throw std::invalid_argument("Array length must be in [1, 1000].");
    }
    for (int v : nums) {
        if (v < 0 || v >= (1 << 16)) {
            throw std::invalid_argument("Array elements must be in [0, 2^16).");
        }
    }
}

bool aliceWins(const std::vector<int>& nums) {
    validateInput(nums);
    int xr = 0;
    for (int v : nums) xr ^= v;
    return xr == 0 || (nums.size() % 2 == 0);
}

static void runTest(const std::vector<int>& nums) {
    try {
        bool res = aliceWins(nums);
        std::cout << "Input: [";
        for (size_t i = 0; i < nums.size(); ++i) {
            std::cout << nums[i] << (i + 1 < nums.size() ? "," : "");
        }
        std::cout << "] -> " << (res ? "true" : "false") << "\n";
    } catch (const std::exception& e) {
        std::cout << "Invalid input: " << e.what() << "\n";
    }
}

int main() {
    // 5 test cases
    runTest({1, 1, 2});    // false
    runTest({0, 1});       // true
    runTest({1, 2, 3});    // true
    runTest({1});          // false
    runTest({0});          // true
    return 0;
}