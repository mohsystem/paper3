#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Helper functions to find max and min of two integers
int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    if (nums1Size > nums2Size) {
        // Recursive call with swapped arrays to ensure the first array is smaller
        return findMedianSortedArrays(nums2, nums2Size, nums1, nums1Size);
    }

    int m = nums1Size;
    int n = nums2Size;
    int low = 0;
    int high = m;

    while (low <= high) {
        int partitionX = low + (high - low) / 2;
        int partitionY = (m + n + 1) / 2 - partitionX;

        int maxX = (partitionX == 0) ? INT_MIN : nums1[partitionX - 1];
        int minX = (partitionX == m) ? INT_MAX : nums1[partitionX];

        int maxY = (partitionY == 0) ? INT_MIN : nums2[partitionY - 1];
        int minY = (partitionY == n) ? INT_MAX : nums2[partitionY];
        
        if (maxX <= minY && maxY <= minX) {
            if ((m + n) % 2 == 0) {
                return (double)(max(maxX, maxY) + min(minX, minY)) / 2.0;
            } else {
                return (double)max(maxX, maxY);
            }
        } else if (maxX > minY) {
            high = partitionX - 1;
        } else {
            low = partitionX + 1;
        }
    }
    
    // Should not be reached if arrays are sorted
    return 0.0;
}

int main() {
    // Test Case 1
    int nums1_1[] = {1, 3};
    int nums2_1[] = {2};
    printf("Test Case 1: %f\n", findMedianSortedArrays(nums1_1, 2, nums2_1, 1));

    // Test Case 2
    int nums1_2[] = {1, 2};
    int nums2_2[] = {3, 4};
    printf("Test Case 2: %f\n", findMedianSortedArrays(nums1_2, 2, nums2_2, 2));

    // Test Case 3
    int nums1_3[] = {0, 0};
    int nums2_3[] = {0, 0};
    printf("Test Case 3: %f\n", findMedianSortedArrays(nums1_3, 2, nums2_3, 2));

    // Test Case 4
    int* nums1_4 = NULL; 
    int nums2_4[] = {1};
    printf("Test Case 4: %f\n", findMedianSortedArrays(nums1_4, 0, nums2_4, 1));
    
    // Test Case 5
    int nums1_5[] = {2};
    int* nums2_5 = NULL; 
    printf("Test Case 5: %f\n", findMedianSortedArrays(nums1_5, 1, nums2_5, 0));

    return 0;
}