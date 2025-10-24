
#include <stdio.h>
#include <limits.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    if (nums1Size > nums2Size) {
        return findMedianSortedArrays(nums2, nums2Size, nums1, nums1Size);
    }
    
    int m = nums1Size;
    int n = nums2Size;
    int low = 0, high = m;
    
    while (low <= high) {
        int partitionX = (low + high) / 2;
        int partitionY = (m + n + 1) / 2 - partitionX;
        
        int maxLeftX = (partitionX == 0) ? INT_MIN : nums1[partitionX - 1];
        int minRightX = (partitionX == m) ? INT_MAX : nums1[partitionX];
        
        int maxLeftY = (partitionY == 0) ? INT_MIN : nums2[partitionY - 1];
        int minRightY = (partitionY == n) ? INT_MAX : nums2[partitionY];
        
        if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
            if ((m + n) % 2 == 0) {
                return (max(maxLeftX, maxLeftY) + min(minRightX, minRightY)) / 2.0;
            } else {
                return max(maxLeftX, maxLeftY);
            }
        } else if (maxLeftX > minRightY) {
            high = partitionX - 1;
        } else {
            low = partitionX + 1;
        }
    }
    
    return 0.0;
}

int main() {
    // Test case 1
    int nums1_1[] = {1, 3};
    int nums2_1[] = {2};
    printf("Test 1: %f\\n", findMedianSortedArrays(nums1_1, 2, nums2_1, 1));
    
    // Test case 2
    int nums1_2[] = {1, 2};
    int nums2_2[] = {3, 4};
    printf("Test 2: %f\\n", findMedianSortedArrays(nums1_2, 2, nums2_2, 2));
    
    // Test case 3
    int nums2_3[] = {1};
    printf("Test 3: %f\\n", findMedianSortedArrays(NULL, 0, nums2_3, 1));
    
    // Test case 4
    int nums1_4[] = {1, 2, 3, 4, 5};
    int nums2_4[] = {6, 7, 8, 9, 10};
    printf("Test 4: %f\\n", findMedianSortedArrays(nums1_4, 5, nums2_4, 5));
    
    // Test case 5
    int nums1_5[] = {1, 3, 5, 7};
    int nums2_5[] = {2, 4, 6};
    printf("Test 5: %f\\n", findMedianSortedArrays(nums1_5, 4, nums2_5, 3));
    
    return 0;
}
