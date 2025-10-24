#include <iostream>
#include <vector>
#include <numeric>
#include <functional>

class Solution {
public:
    bool xorGame(const std::vector<int>& nums) {
        int xorSum = std::accumulate(nums.begin(), nums.end(), 0, std::bit_xor<int>());
        return xorSum == 0 || nums.size() % 2 == 0;
    }
};

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    Solution solver;
    std::cout << std::boolalpha;

    // Test Case 1
    std::vector<int> nums1 = {1, 1, 2};
    std::cout << "Test Case 1: nums = ";
    printVector(nums1);
    std::cout << ", Result: " << solver.xorGame(nums1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {0, 1};
    std::cout << "Test Case 2: nums = ";
    printVector(nums2);
    std::cout << ", Result: " << solver.xorGame(nums2) << std::endl;
    
    // Test Case 3
    std::vector<int> nums3 = {1, 2, 3};
    std::cout << "Test Case 3: nums = ";
    printVector(nums3);
    std::cout << ", Result: " << solver.xorGame(nums3) << std::endl;
    
    // Test Case 4
    std::vector<int> nums4 = {1, 1};
    std::cout << "Test Case 4: nums = ";
    printVector(nums4);
    std::cout << ", Result: " << solver.xorGame(nums4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {1, 2, 3, 4, 5, 6, 7};
    std::cout << "Test Case 5: nums = ";
    printVector(nums5);
    std::cout << ", Result: " << solver.xorGame(nums5) << std::endl;

    return 0;
}