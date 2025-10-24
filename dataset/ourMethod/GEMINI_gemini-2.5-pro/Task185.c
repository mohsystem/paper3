#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

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
                return ((double)max(maxLeftX, maxLeftY) + min(minRightX, minRightY)) / 2.0;
            } else {
                return (double)max(maxLeftX, maxLeftY);
            }
        } else if (maxLeftX > minRightY) {
            high = partitionX - 1;
        } else {
            low = partitionX + 1;
        }
    }

    fprintf(stderr, "Invalid input arrays.\n");
    exit(1);
    
    return -1.0; 
}

int main() {
    // Test Case 1
    int nums1_1[] = {1, 3};
    int nums2_1[] = {2};
    int m1 = sizeof(nums1_1)/sizeof(nums1_1[0]);
    int n1 = sizeof(nums2_1)/sizeof(nums2_1[0]);
    printf("Test Case 1: %f\n", findMedianSortedArrays(nums1_1, m1, nums2_1, n1));

    // Test Case 2
    int nums1_2[] = {1, 2};
    int nums2_2[] = {3, 4};
    int m2 = sizeof(nums1_2)/sizeof(nums1_2[0]);
    int n2 = sizeof(nums2_2)/sizeof(nums2_2[0]);
    printf("Test Case 2: %f\n", findMedianSortedArrays(nums1_2, m2, nums2_2, n2));

    // Test Case 3
    int nums1_3[] = {0, 0};
    int nums2_3[] = {0, 0};
    int m3 = sizeof(nums1_3)/sizeof(nums1_3[0]);
    int n3 = sizeof(nums2_3)/sizeof(nums2_3[0]);
    printf("Test Case 3: %f\n", findMedianSortedArrays(nums1_3, m3, nums2_3, n3));

    // Test Case 4
    int nums1_4[] = {}; 
    int nums2_4[] = {1};
    int m4 = 0;
    int n4 = sizeof(nums2_4)/sizeof(nums2_4[0]);
    printf("Test Case 4: %f\n", findMedianSortedArrays(nums1_4, m4, nums2_4, n4));
    
    // Test Case 5
    int nums1_5[] = {2};
    int nums2_5[] = {};
    int m5 = sizeof(nums1_5)/sizeof(nums1_5[0]);
    int n5 = 0;
    printf("Test Case 5: %f\n", findMedianSortedArrays(nums1_5, m5, nums2_5, n5));

    return 0;
}