#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Determines if Alice wins the XOR game.
 * 
 * Alice wins if:
 * 1. The bitwise XOR of all elements is 0 at the start of her turn.
 * 2. The number of elements is even. If the initial XOR is not 0, she can always
 *    make a move to keep the XOR non-zero. Since the total number of turns is even,
 *    Bob will make the last move, making the XOR sum 0, and lose.
 * 
 * If the number of elements is odd and the initial XOR is not 0, Alice will make
 * the last move and lose.
 * 
 * @param nums A pointer to an array of integers on the chalkboard.
 * @param numsSize The number of elements in the array.
 * @return true if Alice wins, false otherwise.
 */
bool xorGame(int* nums, int numsSize) {
    if (nums == NULL || numsSize < 1) {
        // According to constraints, numsSize is always >= 1.
        // This is a safeguard for invalid inputs.
        return false;
    }

    int xorSum = 0;
    for (int i = 0; i < numsSize; i++) {
        xorSum ^= nums[i];
    }

    // Alice wins if the initial XOR sum is 0 (immediate win)
    // or if the number of elements is even.
    return xorSum == 0 || numsSize % 2 == 0;
}

void print_array(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1: Example 1, Expected: false
    int nums1[] = {1, 1, 2};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    printf("Input: ");
    print_array(nums1, size1);
    printf(", Output: %s\n", xorGame(nums1, size1) ? "true" : "false");

    // Test Case 2: Example 2, Expected: true
    int nums2[] = {0, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    printf("Input: ");
    print_array(nums2, size2);
    printf(", Output: %s\n", xorGame(nums2, size2) ? "true" : "false");

    // Test Case 3: Example 3, Expected: true
    int nums3[] = {1, 2, 3};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("Input: ");
    print_array(nums3, size3);
    printf(", Output: %s\n", xorGame(nums3, size3) ? "true" : "false");

    // Test Case 4: N even, XOR non-zero. Expected: true
    int nums4[] = {1, 2, 3, 6};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("Input: ");
    print_array(nums4, size4);
    printf(", Output: %s\n", xorGame(nums4, size4) ? "true" : "false");

    // Test Case 5: N odd, XOR non-zero. Expected: false
    int nums5[] = {4, 5, 2};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    printf("Input: ");
    print_array(nums5, size5);
    printf(", Output: %s\n", xorGame(nums5, size5) ? "true" : "false");

    return 0;
}