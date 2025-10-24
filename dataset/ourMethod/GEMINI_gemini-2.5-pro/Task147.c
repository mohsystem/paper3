#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Comparison function for qsort (for ascending order).
static int compare_ints(const void *a, const void *b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

/**
 * Finds the kth largest element in an array of integers.
 * This implementation sorts a copy of the array and picks the element at the correct index.
 *
 * @param nums   A pointer to the constant input array of integers.
 * @param size   The number of elements in the array.
 * @param k      The rank of the element to find (1-based index).
 * @param result A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., invalid input or memory allocation error).
 */
int findKthLargest(const int* nums, size_t size, int k, int* result) {
    if (nums == NULL || result == NULL || size == 0 || k <= 0 || (size_t)k > size) {
        return -1; // Invalid arguments
    }

    int* temp_nums = (int*)malloc(size * sizeof(int));
    if (temp_nums == NULL) {
        return -1; // Memory allocation failure
    }

    memcpy(temp_nums, nums, size * sizeof(int));

    qsort(temp_nums, size, sizeof(int), compare_ints);

    *result = temp_nums[size - k];

    free(temp_nums);
    temp_nums = NULL;

    return 0; // Success
}

void run_test_case(int test_num, const int* nums, size_t size, int k, int expected) {
    printf("Test Case %d:\n", test_num);
    printf("Input array: { ");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", nums[i], (i == size - 1 ? "" : ", "));
    }
    printf(" }\n");
    printf("k: %d\n", k);

    int result;
    if (findKthLargest(nums, size, k, &result) == 0) {
        printf("Result: %d\n", result);
        printf("Expected: %d\n", expected);
        printf("Status: %s\n", (result == expected ? "PASSED" : "FAILED"));
    } else {
        fprintf(stderr, "Function failed (invalid input or memory error).\n");
        printf("Status: FAILED (function call failed)\n");
    }
    printf("--------------------\n");
}

int main() {
    int arr1[] = {3, 2, 1, 5, 6, 4};
    run_test_case(1, arr1, sizeof(arr1)/sizeof(arr1[0]), 2, 5);

    int arr2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    run_test_case(2, arr2, sizeof(arr2)/sizeof(arr2[0]), 4, 4);

    int arr3[] = {1};
    run_test_case(3, arr3, sizeof(arr3)/sizeof(arr3[0]), 1, 1);
    
    int arr4[] = {99, 99};
    run_test_case(4, arr4, sizeof(arr4)/sizeof(arr4[0]), 1, 99);
    
    int arr5[] = {-1, -1, -2, -5};
    run_test_case(5, arr5, sizeof(arr5)/sizeof(arr5[0]), 3, -2);
    
    // Additional test case for invalid input
    printf("Test Case 6: Invalid k (k=4 for array of size 3)\n");
    int arr6[] = {1, 2, 3};
    int result6;
    if (findKthLargest(arr6, sizeof(arr6)/sizeof(arr6[0]), 4, &result6) != 0) {
        printf("Caught expected failure for invalid k.\n");
        printf("Status: PASSED\n");
    } else {
        fprintf(stderr, "Did not catch invalid k.\n");
        printf("Status: FAILED\n");
    }
    printf("--------------------\n");

    return 0;
}