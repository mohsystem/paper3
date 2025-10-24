#include <stdio.h>
#include <stdbool.h>

bool xorGame(int* nums, int numsSize) {
    int xor_sum = 0;
    for (int i = 0; i < numsSize; i++) {
        xor_sum ^= nums[i];
    }
    return xor_sum == 0 || numsSize % 2 == 0;
}

int main() {
    // Test Case 1
    int nums1[] = {1, 1, 2};
    printf("%s\n", xorGame(nums1, sizeof(nums1)/sizeof(nums1[0])) ? "true" : "false");

    // Test Case 2
    int nums2[] = {0, 1};
    printf("%s\n", xorGame(nums2, sizeof(nums2)/sizeof(nums2[0])) ? "true" : "false");

    // Test Case 3
    int nums3[] = {1, 2, 3};
    printf("%s\n", xorGame(nums3, sizeof(nums3)/sizeof(nums3[0])) ? "true" : "false");

    // Test Case 4
    int nums4[] = {1, 2, 4, 8};
    printf("%s\n", xorGame(nums4, sizeof(nums4)/sizeof(nums4[0])) ? "true" : "false");

    // Test Case 5
    int nums5[] = {1, 2, 4};
    printf("%s\n", xorGame(nums5, sizeof(nums5)/sizeof(nums5[0])) ? "true" : "false");

    return 0;
}