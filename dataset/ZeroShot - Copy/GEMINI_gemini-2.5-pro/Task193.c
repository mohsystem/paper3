#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Determines if Alice wins the Chalkboard XOR Game.
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

    // Alice wins if she starts with an XOR sum of 0,
    // or if the number of elements is even.
    // If n is even and xorSum != 0, Alice can always make a move
    // such that the remaining xorSum is not 0. Bob will be left
    // with an odd number of elements. The game continues until Bob
    // is forced to take the last element, making the xorSum 0 and losing.
    return xorSum == 0 || numsSize % 2 == 0;
}

void print_array(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

void run_test_case(const char* name, int* nums, int numsSize, bool expected) {
    bool result = xorGame(nums, numsSize);
    printf("%s: nums = ", name);
    print_array(nums, numsSize);
    printf(", Expected: %s, Got: %s\n", expected ? "true" : "false", result ? "true" : "false");
}

int main() {
    // Test Case 1
    int nums1[] = {1, 1, 2};
    run_test_case("Test Case 1", nums1, sizeof(nums1)/sizeof(nums1[0]), false);

    // Test Case 2
    int nums2[] = {0, 1};
    run_test_case("Test Case 2", nums2, sizeof(nums2)/sizeof(nums2[0]), true);

    // Test Case 3
    int nums3[] = {1, 2, 3};
    run_test_case("Test Case 3", nums3, sizeof(nums3)/sizeof(nums3[0]), true);

    // Test Case 4: n is even, xorSum != 0
    int nums4[] = {1, 2, 3, 4}; // xorSum = 4
    run_test_case("Test Case 4", nums4, sizeof(nums4)/sizeof(nums4[0]), true);

    // Test Case 5: n is odd, xorSum != 0
    int nums5[] = {1, 2, 4}; // xorSum = 7
    run_test_case("Test Case 5", nums5, sizeof(nums5)/sizeof(nums5[0]), false);

    return 0;
}