// Task29 - C++ Implementation
// Chain-of-Through process:
// 1) Problem understanding: find two indices i,j such that nums[i] + nums[j] == target.
// 2) Security requirements: avoid undefined behavior, validate basic inputs.
// 3) Secure coding generation: use unordered_map for O(1) expected lookups; no raw pointer arithmetic.
// 4) Code review: bounds-safe, returns pair, no memory leaks.
// 5) Secure code output: final robust implementation with tests.

#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <utility>

std::pair<int, int> two_sum(const std::vector<long long>& nums, long long target) {
    if (nums.size() < 2) {
        throw std::invalid_argument("Input vector must have at least two elements.");
    }
    std::unordered_map<long long, int> seen;
    seen.reserve(nums.size() * 2);
    for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
        long long comp = target - nums[i];
        auto it = seen.find(comp);
        if (it != seen.end() && it->second != i) {
            return {it->second, i};
        }
        seen[nums[i]] = i;
    }
    throw std::runtime_error("No solution found, input violated problem constraints.");
}

static void print_pair(const std::pair<int,int>& p) {
    std::cout << "[" << p.first << ", " << p.second << "]\n";
}

int main() {
    std::pair<int,int> r1 = two_sum(std::vector<long long>{1, 2, 3}, 4);          // e.g., [0,2]
    std::pair<int,int> r2 = two_sum(std::vector<long long>{3, 2, 4}, 6);          // [1,2]
    std::pair<int,int> r3 = two_sum(std::vector<long long>{2, 7, 11, 15}, 9);     // [0,1]
    std::pair<int,int> r4 = two_sum(std::vector<long long>{0, 4, 3, 0}, 0);       // [0,3]
    std::pair<int,int> r5 = two_sum(std::vector<long long>{-3, 4, 3, 90}, 0);     // [0,2]

    print_pair(r1);
    print_pair(r2);
    print_pair(r3);
    print_pair(r4);
    print_pair(r5);

    return 0;
}