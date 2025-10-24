#include <iostream>
#include <vector>

bool xorGame(const std::vector<int>& nums) {
    if (nums.empty()) return true;
    int x = 0;
    for (int v : nums) {
        x ^= v;
    }
    if (x == 0) return true;
    return (nums.size() % 2 == 0);
}

void runTest(const std::vector<int>& nums) {
    std::cout << "[";
    for (size_t i = 0; i < nums.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << nums[i];
    }
    std::cout << "] -> " << (xorGame(nums) ? "true" : "false") << std::endl;
}

int main() {
    runTest({1, 1, 2});
    runTest({0, 1});
    runTest({1, 2, 3});
    runTest({5});
    runTest({1, 2});
    return 0;
}