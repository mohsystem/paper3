#include <iostream>
#include <vector>

class Task193 {
public:
    static bool xorGame(const std::vector<int>& nums) {
        if (nums.empty()) {
            return true;
        }
        int x = 0;
        for (int v : nums) {
            x ^= v;
        }
        if (x == 0) {
            return true;
        }
        return (nums.size() % 2 == 0);
    }
};

int main() {
    std::vector<std::vector<int>> tests = {
        {1, 1, 2},
        {0, 1},
        {1, 2, 3},
        {0},
        {1}
    };
    for (const auto& t : tests) {
        std::cout << (Task193::xorGame(t) ? "true" : "false") << std::endl;
    }
    return 0;
}