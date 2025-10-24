#include <iostream>
#include <vector>
#include <algorithm>

int findKthLargest(std::vector<int>& nums, int k) {
    // Sort the vector in ascending order
    std::sort(nums.begin(), nums.end());
    // The kth largest element is at index size() - k
    return nums[nums.size() - k];
}

int main() {
    // Test Case 1
    std::vector<int> nums1 = {3, 2, 1, 5, 6, 4};
    int k1 = 2;
    std::cout << "Test Case 1: " << findKthLargest(nums1, k1) << std::endl;

    // Test Case 2
    std::vector<int> nums2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int k2 = 4;
    std::cout << "Test Case 2: " << findKthLargest(nums2, k2) << std::endl;

    // Test Case 3
    std::vector<int> nums3 = {1};
    int k3 = 1;
    std::cout << "Test Case 3: " << findKthLargest(nums3, k3) << std::endl;

    // Test Case 4
    std::vector<int> nums4 = {7, 6, 5, 4, 3, 2, 1};
    int k4 = 7;
    std::cout << "Test Case 4: " << findKthLargest(nums4, k4) << std::endl;

    // Test Case 5
    std::vector<int> nums5 = {-1, -1, 0, 2};
    int k5 = 3;
    std::cout << "Test Case 5: " << findKthLargest(nums5, k5) << std::endl;

    return 0;
}