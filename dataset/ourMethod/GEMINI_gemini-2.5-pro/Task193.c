#include <stdio.h>
#include <stdbool.h>

bool xorGame(const int* nums, int numsSize) {
    int xorSum = 0;
    for (int i = 0; i < numsSize; ++i) {
        xorSum ^= nums[i];
    }
    return xorSum == 0 || numsSize % 2 == 0;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]");
}

int main() {
    // Test Case 1
    int nums1[] = {1, 1, 2};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    printf("Test Case 1: nums = ");
    printArray(nums1, size1);
    printf(", Result: %s\n", xorGame(nums1, size1) ? "true" : "false");

    // Test Case 2
    int nums2[] = {0, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    printf("Test Case 2: nums = ");
    printArray(nums2, size2);
    printf(", Result: %s\n", xorGame(nums2, size2) ? "true" : "false");
    
    // Test Case 3
    int nums3[] = {1, 2, 3};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("Test Case 3: nums = ");
    printArray(nums3, size3);
    printf(", Result: %s\n", xorGame(nums3, size3) ? "true" : "false");
    
    // Test Case 4
    int nums4[] = {1, 1};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("Test Case 4: nums = ");
    printArray(nums4, size4);
    printf(", Result: %s\n", xorGame(nums4, size4) ? "true" : "false");

    // Test Case 5
    int nums5[] = {1, 2, 3, 4, 5, 6, 7};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    printf("Test Case 5: nums = ");
    printArray(nums5, size5);
    printf(", Result: %s\n", xorGame(nums5, size5) ? "true" : "false");

    return 0;
}