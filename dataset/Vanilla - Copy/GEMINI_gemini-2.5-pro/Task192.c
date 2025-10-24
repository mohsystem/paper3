#include <stdio.h>
#include <stdbool.h>

/**
 * Determines if Alice wins the XOR game.
 *
 * Alice wins if:
 * 1. The bitwise XOR of all numbers on the board is 0 at the start of her turn.
 * 2. The number of elements on the board is even. If the XOR sum is not 0,
 *    she can always make a move that doesn't result in an XOR sum of 0,
 *    passing the turn to Bob. Since Alice always faces an even number of
 *    elements and Bob an odd number, Bob is the one who might be forced
 *    into a losing position.
 *
 * @param nums The array of integers on the chalkboard.
 * @param numsSize The size of the array.
 * @return true if Alice wins, false otherwise.
 */
bool xorGame(int* nums, int numsSize) {
    int xorSum = 0;
    for (int i = 0; i < numsSize; ++i) {
        xorSum ^= nums[i];
    }
    return xorSum == 0 || numsSize % 2 == 0;
}

int main() {
    // Test Case 1
    int nums1[] = {1, 1, 2};
    printf("%s\n", xorGame(nums1, 3) ? "true" : "false"); // Expected: false

    // Test Case 2
    int nums2[] = {0, 1};
    printf("%s\n", xorGame(nums2, 2) ? "true" : "false"); // Expected: true

    // Test Case 3
    int nums3[] = {1, 2, 3};
    printf("%s\n", xorGame(nums3, 3) ? "true" : "false"); // Expected: true

    // Test Case 4
    int nums4[] = {6, 6, 6, 6};
    printf("%s\n", xorGame(nums4, 4) ? "true" : "false"); // Expected: true

    // Test Case 5
    int nums5[] = {5, 3, 2, 1};
    printf("%s\n", xorGame(nums5, 4) ? "true" : "false"); // Expected: true

    return 0;
}