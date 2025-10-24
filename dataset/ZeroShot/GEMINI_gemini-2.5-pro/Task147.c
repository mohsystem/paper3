#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Comparison function for qsort to sort in descending order.
 * This implementation is safe from integer overflow.
 */
int compare_desc(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);
    if (int_a < int_b) return 1;
    if (int_a > int_b) return -1;
    return 0;
}

/**
 * @brief Finds the kth largest element in an array of integers by sorting.
 * @note This function modifies the input array by sorting it. If the original
 *       array must be preserved, pass a copy.
 *
 * @param nums     The input array of integers.
 * @param numsSize The size of the array.
 * @param k        The 'k' value, representing the desired largest element rank.
 * @param result   A pointer to an integer where the result will be stored.
 * @return         0 on success, -1 on failure (invalid input).
 */
int findKthLargest(int* nums, int numsSize, int k, int* result) {
    // 1. Input Validation
    if (nums == NULL || result == NULL || numsSize <= 0 || k < 1 || k > numsSize) {
        return -1; // Indicate error
    }

    // 2. Sort the array in descending order. qsort modifies the array in-place.
    qsort(nums, numsSize, sizeof(int), compare_desc);

    // 3. The kth largest element is at index k-1 in the sorted array.
    *result = nums[k - 1];

    return 0; // Indicate success
}

void run_test(const char* name, int* original_nums, int numsSize, int k) {
    printf("%s: Array = [", name);
    for (int i = 0; i < numsSize; ++i) {
        printf("%d%s", original_nums[i], (i == numsSize - 1 ? "" : ", "));
    }
    printf("], k = %d", k);

    // Since the function modifies the array, we create a copy for it to use.
    int* nums_copy = (int*)malloc(numsSize * sizeof(int));
    if (numsSize > 0 && nums_copy == NULL) {
        printf("\nMemory allocation failed for test case.\n");
        return;
    }
    for(int i = 0; i < numsSize; ++i) {
        nums_copy[i] = original_nums[i];
    }
    
    int actual_result;
    if (findKthLargest(nums_copy, numsSize, k, &actual_result) == 0) {
        printf(" -> Result: %d\n", actual_result);
    } else {
        printf("\nCaught expected error from function.\n");
    }

    free(nums_copy);
}

int main() {
    // Test Case 1
    int nums1[] = {3, 2, 1, 5, 6, 4};
    run_test("Test Case 1", nums1, 6, 2);

    // Test Case 2
    int nums2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    run_test("Test Case 2", nums2, 9, 4);

    // Test Case 3
    int nums3[] = {1};
    run_test("Test Case 3", nums3, 1, 1);

    // Test Case 4
    int nums4[] = {7, 6, 5, 4, 3, 2, 1};
    run_test("Test Case 4", nums4, 7, 7);

    // Test Case 5
    int nums5[] = {99, 99};
    run_test("Test Case 5", nums5, 2, 1);
    
    // Invalid Input Test
    int nums6[] = {};
    run_test("Invalid Input Test", nums6, 0, 1);

    return 0;
}