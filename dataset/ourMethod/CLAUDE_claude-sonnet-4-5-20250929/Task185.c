
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Macro for safe minimum of two values
#define MIN(a, b) ((a) < (b) ? (a) : (b))
// Macro for safe maximum of two values
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Function to find median of two sorted arrays with O(log(m+n)) complexity
// Returns median value, or -1.0 on error (caller should check errno or validate inputs)
double findMedianSortedArrays(const int* nums1, size_t m, const int* nums2, size_t n) {
    // Input validation: check for NULL pointers when size > 0
    if ((m > 0 && nums1 == NULL) || (n > 0 && nums2 == NULL)) {
        return -1.0; // Error indicator
    }
    
    // Validate size constraints
    if (m > 1000 || n > 1000 || (m + n) == 0 || (m + n) > 2000) {
        return -1.0; // Error indicator
    }
    
    // Ensure nums1 is the smaller array for efficiency and safety
    if (m > n) {
        return findMedianSortedArrays(nums2, n, nums1, m);
    }
    
    size_t total = m + n;
    size_t left = 0;
    size_t right = m;
    
    // Binary search on the smaller array
    while (left <= right) {
        // Safe partition calculation to prevent overflow
        size_t partitionX = left + (right - left) / 2;
        
        // Calculate partitionY with overflow check
        size_t half = (m + n + 1) / 2;
        if (partitionX > half) {
            return -1.0; // Invalid state
        }
        size_t partitionY = half - partitionX;
        
        // Boundary validation to prevent out-of-bounds access
        if (partitionY > n) {
            if (partitionX < m) {
                left = partitionX + 1;
            } else {
                break;
            }
            continue;
        }
        
        // Get boundary elements with safe bounds checking
        int maxLeftX = (partitionX == 0) ? INT_MIN : nums1[partitionX - 1];
        int minRightX = (partitionX == m) ? INT_MAX : nums1[partitionX];
        
        int maxLeftY = (partitionY == 0) ? INT_MIN : nums2[partitionY - 1];
        int minRightY = (partitionY == n) ? INT_MAX : nums2[partitionY];
        
        // Check if we found the correct partition
        if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
            // Calculate median based on odd/even total length
            if (total % 2 == 0) {
                // Even: average of two middle elements
                int left_max = MAX(maxLeftX, maxLeftY);
                int right_min = MIN(minRightX, minRightY);
                return (left_max + right_min) / 2.0;
            } else {
                // Odd: the middle element
                return (double)MAX(maxLeftX, maxLeftY);
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
    
    return -1.0; // Error: arrays not sorted or invalid
}

int main(void) {
    // Test case 1: Example from problem
    int nums1_1[] = {1, 3};
    int nums2_1[] = {2};
    double result1 = findMedianSortedArrays(nums1_1, 2, nums2_1, 1);
    printf("Test 1: %.5f\\n", result1);
    
    // Test case 2: Example from problem
    int nums1_2[] = {1, 2};
    int nums2_2[] = {3, 4};
    double result2 = findMedianSortedArrays(nums1_2, 2, nums2_2, 2);
    printf("Test 2: %.5f\\n", result2);
    
    // Test case 3: One empty array
    int nums2_3[] = {1, 2, 3, 4, 5};
    double result3 = findMedianSortedArrays(NULL, 0, nums2_3, 5);
    printf("Test 3: %.5f\\n", result3);
    
    // Test case 4: Negative numbers
    int nums1_4[] = {-5, -3, -1};
    int nums2_4[] = {-2, 0, 2};
    double result4 = findMedianSortedArrays(nums1_4, 3, nums2_4, 3);
    printf("Test 4: %.5f\\n", result4);
    
    // Test case 5: Single element in each
    int nums1_5[] = {1};
    int nums2_5[] = {2};
    double result5 = findMedianSortedArrays(nums1_5, 1, nums2_5, 1);
    printf("Test 5: %.5f\\n", result5);
    
    return 0;
}
