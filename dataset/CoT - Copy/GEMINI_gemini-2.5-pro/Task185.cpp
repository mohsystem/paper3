#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

class Task185 {
public:
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

            int maxX = (partitionX == 0) ? std::numeric_limits<int>::min() : nums1[partitionX - 1];
            int minX = (partitionX == m) ? std::numeric_limits<int>::max() : nums1[partitionX];

            int maxY = (partitionY == 0) ? std::numeric_limits<int>::min() : nums2[partitionY - 1];
            int minY = (partitionY == n) ? std::numeric_limits<int>::max() : nums2[partitionY];
            
            if (maxX <= minY && maxY <= minX) {
                if ((m + n) % 2 == 0) {
                    return (double)(std::max(maxX, maxY) + std::min(minX, minY)) / 2.0;
                } else {
                    return (double)std::max(maxX, maxY);
                }
            } else if (maxX > minY) {
                high = partitionX - 1;
            } else {
                low = partitionX + 1;
            }
        }
        
        return 0.0; // Should not be reached
    }
};

int main() {
    Task185 solution;
    
    // Test Case 1
    std::vector<int> nums1_1 = {1, 3};
    std::vector<int> nums2_1 = {2};
    std::cout << "Test Case 1: " << solution.findMedianSortedArrays(nums1_1, nums2_1) << std::endl;

    // Test Case 2
    std::vector<int> nums1_2 = {1, 2};
    std::vector<int> nums2_2 = {3, 4};
    std::cout << "Test Case 2: " << solution.findMedianSortedArrays(nums1_2, nums2_2) << std::endl;
    
    // Test Case 3
    std::vector<int> nums1_3 = {0, 0};
    std::vector<int> nums2_3 = {0, 0};
    std::cout << "Test Case 3: " << solution.findMedianSortedArrays(nums1_3, nums2_3) << std::endl;

    // Test Case 4
    std::vector<int> nums1_4 = {};
    std::vector<int> nums2_4 = {1};
    std::cout << "Test Case 4: " << solution.findMedianSortedArrays(nums1_4, nums2_4) << std::endl;
    
    // Test Case 5
    std::vector<int> nums1_5 = {2};
    std::vector<int> nums2_5 = {};
    std::cout << "Test Case 5: " << solution.findMedianSortedArrays(nums1_5, nums2_5) << std::endl;
    
    return 0;
}