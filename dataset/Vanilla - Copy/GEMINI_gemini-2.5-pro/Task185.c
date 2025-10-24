#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    if (nums1Size > nums2Size) {
        return findMedianSortedArrays(nums2, nums2Size, nums1, nums1Size);
    }

    int m = nums1Size;
    int n = nums2Size;
    int low = 0;
    int high = m;

    while (low <= high) {
        int partitionX = low + (high - low) / 2;
        int partitionY = (m + n + 1) / 2 - partitionX;

        int maxLeftX = (partitionX == 0) ? INT_MIN : nums1[partitionX - 1];
        int minRightX = (partitionX == m) ? INT_MAX : nums1[partitionX];

        int maxLeftY = (partitionY == 0) ? INT_MIN : nums2[partitionY - 1];
        int minRightY = (partitionY == n) ? INT_MAX : nums2[partitionY];

        if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
            if ((m + n) % 2 == 0) {
                return (max(maxLeftX, maxLeftY) + min(minRightX, minRightY)) / 2.0;
            } else {
                return (double)max(maxLeftX, maxLeftY);
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
    // Test Case 1
    int nums1_1[] = {1, 3};
    int nums1_2[] = {2};
    printf("Test Case 1: %.5f\n", findMedianSortedArrays(nums1_1, 2, nums1_2, 1));

    // Test Case 2
    int nums2_1[] = {1, 2};
    int nums2_2[] = {3, 4};
    printf("Test Case 2: %.5f\n", findMedianSortedArrays(nums2_1, 2, nums2_2, 2));

    // Test Case 3
    int nums3_1[] = {0, 0};
    int nums3_2[] = {0, 0};
    printf("Test Case 3: %.5f\n", findMedianSortedArrays(nums3_1, 2, nums3_2, 2));

    // Test Case 4
    int* nums4_1 = NULL;
    int nums4_2[] = {1};
    printf("Test Case 4: %.5f\n", findMedianSortedArrays(nums4_1, 0, nums4_2, 1));

    // Test Case 5
    int nums5_1[] = {2};
    int* nums5_2 = NULL;
    printf("Test Case 5: %.5f\n", findMedianSortedArrays(nums5_1, 1, nums5_2, 0));

    return 0;
}