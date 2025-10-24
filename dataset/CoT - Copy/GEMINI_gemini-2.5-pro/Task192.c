#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Determines if Alice wins the XOR game.
 *
 * The logic is based on a game theory analysis:
 * A player wins if they start their turn and the XOR sum of all numbers is 0.
 * A player loses if any move they make results in an XOR sum of 0.
 * An optimal player will always make a move to a state that is a losing position for the opponent, if possible.
 *
 * Let's analyze the game based on the number of elements N.
 * Case 1: The initial XOR sum of all numbers is 0.
 * Alice starts with an XOR sum of 0, so she wins immediately.
 *
 * Case 2: The initial XOR sum is not 0.
 *   - If N is even: Alice can always choose a number `x` to remove such that the new XOR sum is not 0.
 *     This leaves Bob with N-1 (an odd number) elements and a non-zero XOR sum.
 *     A game state with an odd number of elements and a non-zero XOR sum is a losing position.
 *     Therefore, Alice can force a win.
 *   - If N is odd: Any number Alice removes will leave N-1 (an even number) elements for Bob.
 *     A game state with an even number of elements is always a winning position for the player whose turn it is (Bob in this case).
 *     Therefore, Alice cannot win.
 *
 * Combining these, Alice wins if the initial XOR sum is 0, OR if the number of elements is even.
 *
 * @param nums The array of integers on the chalkboard.
 * @param numsSize The size of the array.
 * @return true if Alice wins, false otherwise.
 */
bool xorGame(int* nums, int numsSize) {
    int xorSum = 0;
    for (int i = 0; i < numsSize; i++) {
        xorSum ^= nums[i];
    }
    return xorSum == 0 || numsSize % 2 == 0;
}

void print_array(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]");
}

int main() {
    // Test Case 1
    int nums1[] = {1, 1, 2};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    printf("Test Case 1: nums = ");
    print_array(nums1, size1);
    printf("\n");
    printf("Output: %s\n", xorGame(nums1, size1) ? "true" : "false"); // Expected: false

    // Test Case 2
    int nums2[] = {0, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    printf("\nTest Case 2: nums = ");
    print_array(nums2, size2);
    printf("\n");
    printf("Output: %s\n", xorGame(nums2, size2) ? "true" : "false"); // Expected: true

    // Test Case 3
    int nums3[] = {1, 2, 3};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("\nTest Case 3: nums = ");
    print_array(nums3, size3);
    printf("\n");
    printf("Output: %s\n", xorGame(nums3, size3) ? "true" : "false"); // Expected: true

    // Test Case 4
    int nums4[] = {1, 1};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("\nTest Case 4: nums = ");
    print_array(nums4, size4);
    printf("\n");
    printf("Output: %s\n", xorGame(nums4, size4) ? "true" : "false"); // Expected: true

    // Test Case 5
    int nums5[] = {5, 4, 3, 2, 1};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    printf("\nTest Case 5: nums = ");
    print_array(nums5, size5);
    printf("\n");
    printf("Output: %s\n", xorGame(nums5, size5) ? "true" : "false"); // Expected: false

    return 0;
}