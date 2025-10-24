#include <iostream>
#include <vector>
#include <numeric>
#include <functional>

class Task193 {
public:
    bool xorGame(const std::vector<int>& nums) {
        int xor_sum = std::accumulate(nums.begin(), nums.end(), 0, std::bit_xor<int>());
        return xor_sum == 0 || nums.size() % 2 == 0;
    }
};

int main() {
    Task193 solver;
    
    // Test Case 1
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << std::boolalpha << solver.xorGame(nums1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {0, 1};
    std::cout << std::boolalpha << solver.xorGame(nums2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << std::boolalpha << solver.xorGame(nums3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {1, 2, 4, 8};
    std::cout << std::boolalpha << solver.xorGame(nums4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {1, 2, 4};
    std::cout << std::boolalpha << solver.xorGame(nums5) << std::endl;
    
    return 0;
}