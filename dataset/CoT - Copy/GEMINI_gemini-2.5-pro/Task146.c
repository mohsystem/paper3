#include <stdio.h>

/**
 * @brief Finds the missing number in an array containing n-1 unique integers
 * from the range [1, n].
 * 
 * @param nums Pointer to the input array of unique integers.
 * @param size The number of elements in the array.
 * @return The missing integer.
 */
int findMissingNumber(const int* nums, int size) {
    if (nums == NULL && size > 0) {
        // Handle invalid input
        return 0;
    }
    
    // 'n' is the expected size of the complete sequence
    int n = size + 1;
    
    // Use long long to prevent potential integer overflow for large n
    long long expectedSum = (long long)n * (n + 1) / 2;
    
    long long actualSum = 0;
    for (int i = 0; i < size; i++) {
        actualSum += nums[i];
    }
    
    // The difference is the missing number
    return (int)(expectedSum - actualSum);
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 4, 5};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1: Input: [1, 2, 4, 5], Missing Number: %d\n", findMissingNumber(arr1, size1));

    // Test Case 2
    int arr2[] = {2, 3, 1, 5};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2: Input: [2, 3, 1, 5], Missing Number: %d\n", findMissingNumber(arr2, size2));

    // Test Case 3
    int arr3[] = {1};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3: Input: [1], Missing Number: %d\n", findMissingNumber(arr3, size3));

    // Test Case 4
    int arr4[] = {2};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4: Input: [2], Missing Number: %d\n", findMissingNumber(arr4, size4));
    
    // Test Case 5 (Edge case: empty array)
    int arr5[] = {};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5: Input: [], Missing Number: %d\n", findMissingNumber(arr5, size5));

    return 0;
}