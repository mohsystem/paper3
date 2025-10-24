#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>

double findMedianSortedArrays(std::vector<int>& nums1, std::vector<int>& nums2) {
    if (nums1.size() > nums2.size()) {
        return findMedianSortedArrays(nums2, nums1);
    }

    int m = nums1.size();
    int n = nums2.size();
    int low = 0;
    int high = m;

    while (low <= high) {
        int partitionX = low + (high - low) / 2;
        int partitionY = (m + n + 1) / 2 - partitionX;

        int maxLeftX = (partitionX == 0) ? std::numeric_limits<int>::min() : nums1[partitionX - 1];
        int minRightX = (partitionX == m) ? std::numeric_limits<int>::max() : nums1[partitionX];

        int maxLeftY = (partitionY == 0) ? std::numeric_limits<int>::min() : nums2[partitionY - 1];
        int minRightY = (partitionY == n) ? std::numeric_limits<int>::max() : nums2[partitionY];

        if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
            if ((m + n) % 2 == 0) {
                return (std::max(maxLeftX, maxLeftY) + std::min(minRightX, minRightY)) / 2.0;
            } else {
                return static_cast<double>(std::max(maxLeftX, maxLeftY));
            }
        } else if (maxLeftX > minRightY) {
            high = partitionX - 1;
        } else {
            low = partitionX + 1;
        }
    }
    
    throw std::invalid_argument("Input arrays are invalid.");
}

int main() {
    // Test Case 1
    std::vector<int> nums1_1 = {1, 3};
    std::vector<int> nums2_1 = {2};
    std::cout << "Test Case 1: " << findMedianSortedArrays(nums1_1, nums2_1) << std::endl;

    // Test Case 2
    std::vector<int> nums1_2 = {1, 2};
    std::vector<int> nums2_2 = {3, 4};
    std::cout << "Test Case 2: " << findMedianSortedArrays(nums1_2, nums2_2) << std::endl;

    // Test Case 3
    std::vector<int> nums1_3 = {0, 0};
    std::vector<int> nums2_3 = {0, 0};
    std::cout << "Test Case 3: " << findMedianSortedArrays(nums1_3, nums2_3) << std::endl;

    // Test Case 4
    std::vector<int> nums1_4 = {};
    std::vector<int> nums2_4 = {1};
    std::cout << "Test Case 4: " << findMedianSortedArrays(nums1_4, nums2_4) << std::endl;

    // Test Case 5
    std::vector<int> nums1_5 = {2};
    std::vector<int> nums2_5 = {};
    std::cout << "Test Case 5: " << findMedianSortedArrays(nums1_5, nums2_5) << std::endl;
    
    return 0;
}