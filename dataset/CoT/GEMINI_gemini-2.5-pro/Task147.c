#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // For INT_MIN

// Comparison function required by qsort for sorting integers in ascending order
int compare_integers(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);
    if (int_a == int_b) return 0;
    return (int_a < int_b) ? -1 : 1;
}

/**
 * Finds the kth largest element in an array of integers.
 * Note: This function modifies the input array by sorting it in-place.
 *
 * @param nums The input array of integers.
 * @param size The number of elements in the array.
 * @param k The position 'k' of the largest element to find (1-based index).
 * @return The kth largest element, or INT_MIN as a sentinel value if the input is invalid.
 */
int findKthLargest(int* nums, int size, int k) {
    // Security: Validate inputs to prevent memory errors and handle invalid arguments.
    if (nums == NULL || size <= 0 || k <= 0 || k > size) {
        return INT_MIN; // Use a sentinel value to indicate an error
    }

    // Sort the array in-place in ascending order using quicksort.
    // Time complexity is O(N log N) on average.
    qsort(nums, size, sizeof(int), compare_integers);

    // The kth largest element is at index (size - k) in a 0-indexed array
    // sorted in ascending order.
    return nums[size - k];
}

void print_array(const char* name, int* arr, int size, int k) {
    printf("%s: Array = [", name);
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i < size - 1) ? ", " : "");
    }
    printf("], k = %d\n", k);
}

int main() {
    // Test Case 1: General case
    int nums1[] = {3, 2, 1, 5, 6, 4};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    print_array("Test Case 1", nums1, size1, 2);
    int result1 = findKthLargest(nums1, size1, 2);
    printf("Result: %d\n\n", result1); // Expected: 5

    // Test Case 2: With duplicates
    int nums2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    print_array("Test Case 2", nums2, size2, 4);
    int result2 = findKthLargest(nums2, size2, 4);
    printf("Result: %d\n\n", result2); // Expected: 4

    // Test Case 3: k = 1 (the largest element)
    int nums3[] = {7, 6, 5, 4, 3, 2, 1};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    print_array("Test Case 3", nums3, size3, 1);
    int result3 = findKthLargest(nums3, size3, 1);
    printf("Result: %d\n\n", result3); // Expected: 7

    // Test Case 4: k = n (the smallest element)
    int nums4[] = {7, 6, 5, 4, 3, 2, 1};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    print_array("Test Case 4", nums4, size4, 7);
    int result4 = findKthLargest(nums4, size4, 7);
    printf("Result: %d\n\n", result4); // Expected: 1

    // Test Case 5: Invalid k (k > size of array)
    int nums5[] = {1, 2};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    print_array("Test Case 5", nums5, size5, 3);
    int result5 = findKthLargest(nums5, size5, 3);
    if (result5 == INT_MIN) {
        printf("Result: Invalid input\n\n"); // Expected
    } else {
        printf("Result: %d\n\n", result5);
    }
    
    return 0;
}