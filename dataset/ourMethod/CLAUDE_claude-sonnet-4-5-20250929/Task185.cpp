
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>

// Function to find median of two sorted arrays with O(log(m+n)) complexity
// Uses binary search approach on the smaller array
double findMedianSortedArrays(const std::vector<int>& nums1, const std::vector<int>& nums2) {
    // Input validation: check sizes are within constraints
    if (nums1.size() > 1000 || nums2.size() > 1000) {
        throw std::invalid_argument("Array size exceeds maximum allowed (1000)");
    }
    
    size_t total = nums1.size() + nums2.size();
    if (total == 0 || total > 2000) {
        throw std::invalid_argument("Total size must be between 1 and 2000");
    }
    
    // Ensure nums1 is the smaller array for efficiency
    // This prevents potential out-of-bounds access in binary search
    if (nums1.size() > nums2.size()) {
        return findMedianSortedArrays(nums2, nums1);
    }
    
    size_t m = nums1.size();
    size_t n = nums2.size();
    
    // Binary search on the smaller array
    size_t left = 0;
    size_t right = m;
    
    while (left <= right) {
        // Partition points - using size_t to prevent integer overflow
        size_t partitionX = left + (right - left) / 2;
        size_t partitionY = (m + n + 1) / 2 - partitionX;
        
        // Boundary validation to prevent out-of-bounds access
        if (partitionY > n) {
            // partitionX is too small, search right half
            if (partitionX < m) {
                left = partitionX + 1;
            } else {
                break;
            }
            continue;
        }
        
        // Get boundary elements with safe bounds checking
        int maxLeftX = (partitionX == 0) ? std::numeric_limits<int>::min() : nums1[partitionX - 1];
        int minRightX = (partitionX == m) ? std::numeric_limits<int>::max() : nums1[partitionX];
        
        int maxLeftY = (partitionY == 0) ? std::numeric_limits<int>::min() : nums2[partitionY - 1];
        int minRightY = (partitionY == n) ? std::numeric_limits<int>::max() : nums2[partitionY];
        
        // Check if we found the correct partition
        if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
            // Calculate median based on odd/even total length
            if (total % 2 == 0) {
                // Even: average of two middle elements
                return (std::max(maxLeftX, maxLeftY) + std::min(minRightX, minRightY)) / 2.0;
            } else {
                // Odd: the middle element
                return std::max(maxLeftX, maxLeftY);
            }
        } else if (maxLeftX > minRightY) {
            // Move partition left in nums1
            if (partitionX > 0) {
                right = partitionX - 1;
            } else {
                break;
            }
        } else {
            // Move partition right in nums1
            if (partitionX < m) {
                left = partitionX + 1;
            } else {
                break;
            }
        }
    }
    
    throw std::runtime_error("Input arrays are not sorted or invalid");
}

int main() {
    // Test case 1: Example from problem
    std::vector<int> nums1_1 = {1, 3};
    std::vector<int> nums2_1 = {2};
    std::cout << "Test 1: " << findMedianSortedArrays(nums1_1, nums2_1) << std::endl;
    
    // Test case 2: Example from problem
    std::vector<int> nums1_2 = {1, 2};
    std::vector<int> nums2_2 = {3, 4};
    std::cout << "Test 2: " << findMedianSortedArrays(nums1_2, nums2_2) << std::endl;
    
    // Test case 3: One empty array
    std::vector<int> nums1_3 = {};
    std::vector<int> nums2_3 = {1, 2, 3, 4, 5};
    std::cout << "Test 3: " << findMedianSortedArrays(nums1_3, nums2_3) << std::endl;
    
    // Test case 4: Negative numbers
    std::vector<int> nums1_4 = {-5, -3, -1};
    std::vector<int> nums2_4 = {-2, 0, 2};
    std::cout << "Test 4: " << findMedianSortedArrays(nums1_4, nums2_4) << std::endl;
    
    // Test case 5: Single element in each
    std::vector<int> nums1_5 = {1};
    std::vector<int> nums2_5 = {2};
    std::cout << "Test 5: " << findMedianSortedArrays(nums1_5, nums2_5) << std::endl;
    
    return 0;
}
