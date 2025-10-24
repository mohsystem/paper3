#include <stdio.h>
#include <stdlib.h>

// Comparison function for qsort
int compare_ints(const void* a, const void* b) {
    int int_a = *((int*)a);
    int int_b = *((int*)b);

    if (int_a == int_b) return 0;
    else if (int_a < int_b) return -1;
    else return 1;
}

int findKthLargest(int nums[], int size, int k) {
    // Sort the array using qsort for ascending order
    qsort(nums, size, sizeof(int), compare_ints);
    // The kth largest element is at index size - k
    return nums[size - k];
}

int main() {
    // Test Case 1
    int nums1[] = {3, 2, 1, 5, 6, 4};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    int k1 = 2;
    printf("Test Case 1: %d\n", findKthLargest(nums1, size1, k1));

    // Test Case 2
    int nums2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    int k2 = 4;
    printf("Test Case 2: %d\n", findKthLargest(nums2, size2, k2));

    // Test Case 3
    int nums3[] = {1};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    int k3 = 1;
    printf("Test Case 3: %d\n", findKthLargest(nums3, size3, k3));

    // Test Case 4
    int nums4[] = {7, 6, 5, 4, 3, 2, 1};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    int k4 = 7;
    printf("Test Case 4: %d\n", findKthLargest(nums4, size4, k4));

    // Test Case 5
    int nums5[] = {-1, -1, 0, 2};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    int k5 = 3;
    printf("Test Case 5: %d\n", findKthLargest(nums5, size5, k5));

    return 0;
}