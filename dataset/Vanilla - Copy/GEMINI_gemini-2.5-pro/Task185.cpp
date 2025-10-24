#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <iomanip>

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
    
    // This part should not be reached if inputs are valid sorted arrays
    return -1.0;
}

int main() {
    std::cout << std::fixed << std::setprecision(5);

    // Test Case 1
    std::vector<int> nums1_1 = {1, 3};
    std::vector<int> nums1_2 = {2};
    std::cout << "Test Case 1: " << findMedianSortedArrays(nums1_1, nums1_2) << std::endl;

    // Test Case 2
    std::vector<int> nums2_1 = {1, 2};
    std::vector<int> nums2_2 = {3, 4};
    std::cout << "Test Case 2: " << findMedianSortedArrays(nums2_1, nums2_2) << std::endl;

    // Test Case 3
    std::vector<int> nums3_1 = {0, 0};
    std::vector<int> nums3_2 = {0, 0};
    std::cout << "Test Case 3: " << findMedianSortedArrays(nums3_1, nums3_2) << std::endl;

    // Test Case 4
    std::vector<int> nums4_1 = {};
    std::vector<int> nums4_2 = {1};
    std::cout << "Test Case 4: " << findMedianSortedArrays(nums4_1, nums4_2) << std::endl;

    // Test Case 5
    std::vector<int> nums5_1 = {2};
    std::vector<int> nums5_2 = {};
    std::cout << "Test Case 5: " << findMedianSortedArrays(nums5_1, nums5_2) << std::endl;

    return 0;
}