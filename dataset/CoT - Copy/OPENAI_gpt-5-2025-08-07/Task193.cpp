#include <iostream>
#include <vector>
#include <string>

bool xorGame(const std::vector<int>& nums) {
    int x = 0;
    for (int v : nums) x ^= v;
    if (x == 0) return true;
    return (nums.size() % 2 == 0);
}

static std::string boolStr(bool b) { return b ? "true" : "false"; }

int main() {
    std::vector<std::vector<int>> tests = {
        {1, 1, 2},  // false
        {0, 1},     // true
        {1, 2, 3},  // true
        {5},        // false
        {2, 2}      // true
    };
    for (const auto& t : tests) {
        std::cout << "Input: [";
        for (size_t i = 0; i < t.size(); ++i) {
            if (i) std::cout << ", ";
            std::cout << t[i];
        }
        std::cout << "] -> " << boolStr(xorGame(t)) << "\n";
    }
    return 0;
}