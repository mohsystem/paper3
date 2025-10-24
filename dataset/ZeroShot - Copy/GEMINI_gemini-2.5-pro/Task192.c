#include <stdio.h>
#include <stdbool.h>

/**
 * Determines if Alice wins the XOR game.
 *
 * Alice wins if:
 * 1. The initial bitwise XOR of all numbers is 0. She wins on her first turn.
 * 2. The number of elements is even. In this case, if the initial XOR sum is not 0,
 *    Alice can always make a move that does not result in an XOR sum of 0.
 *    This is because for a player to be forced to lose, they must face a board
 *    where all elements are identical to the current XOR sum, which is only possible
 *    if the number of elements is odd. Since Alice always starts her turn with an
 *    even number of elements, she can never be forced to lose. The game will proceed
 *    to the end, and Bob will take the last turn, making him lose.
 *
 * @param nums The array of integers on the chalkboard.
 * @param numsSize The number of elements in the array.
 * @return true if Alice wins, false otherwise.
 */
bool xorGame(int* nums, int numsSize) {
    int xorSum = 0;
    for (int i = 0; i < numsSize; i++) {
        xorSum ^= nums[i];
    }

    if (xorSum == 0) {
        return true;
    }

    return numsSize % 2 == 0;
}

// Helper function to print test results
void run_test_case(const char* name, int* arr, int size) {
    bool result = xorGame(arr, size);
    printf("%s: %s\n", name, result ? "true" : "false");
}

int main() {
    // Test Case 1: nums = [1,1,2], Output: false
    int nums1[] = {1, 1, 2};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    run_test_case("Test Case 1 for [1, 1, 2]", nums1, size1);

    // Test Case 2: nums = [0,1], Output: true
    int nums2[] = {0, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    run_test_case("Test Case 2 for [0, 1]", nums2, size2);

    // Test Case 3: nums = [1,2,3], Output: true
    int nums3[] = {1, 2, 3};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    run_test_case("Test Case 3 for [1, 2, 3]", nums3, size3);

    // Test Case 4: n is even, xorSum != 0 -> true
    int nums4[] = {1, 2, 4, 8};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    run_test_case("Test Case 4 for [1, 2, 4, 8]", nums4, size4);

    // Test Case 5: n is odd, xorSum != 0 -> false
    int nums5[] = {5, 5, 5};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    run_test_case("Test Case 5 for [5, 5, 5]", nums5, size5);

    return 0;
}