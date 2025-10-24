
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
 * Finds the contiguous subarray with maximum sum using Kadane's algorithm\n * @param arr input array of integers\n * @param size size of the array\n * @param result pointer to store the result\n * @return 0 on success, -1 on error\n */\nint maxSubarraySum(const int* arr, int size, int* result) {\n    // Input validation\n    if (arr == NULL || size <= 0 || result == NULL) {\n        return -1;\n    }\n    \n    int maxSum = arr[0];\n    int currentSum = arr[0];\n    \n    // Kadane's algorithm
    for (int i = 1; i < size; i++) {
        currentSum = (arr[i] > currentSum + arr[i]) ? arr[i] : currentSum + arr[i];
        maxSum = (maxSum > currentSum) ? maxSum : currentSum;
    }
    
    *result = maxSum;
    return 0;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    int result;
    
    // Test case 1: Mixed positive and negative numbers
    int test1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: ");
    printArray(test1, size1);
    if (maxSubarraySum(test1, size1, &result) == 0) {
        printf("Maximum subarray sum: %d\\n", result);
    }
    printf("\\n");
    
    // Test case 2: All negative numbers
    int test2[] = {-5, -2, -8, -1, -4};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: ");
    printArray(test2, size2);
    if (maxSubarraySum(test2, size2, &result) == 0) {
        printf("Maximum subarray sum: %d\\n", result);
    }
    printf("\\n");
    
    // Test case 3: All positive numbers
    int test3[] = {1, 2, 3, 4, 5};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: ");
    printArray(test3, size3);
    if (maxSubarraySum(test3, size3, &result) == 0) {
        printf("Maximum subarray sum: %d\\n", result);
    }
    printf("\\n");
    
    // Test case 4: Single element
    int test4[] = {10};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: ");
    printArray(test4, size4);
    if (maxSubarraySum(test4, size4, &result) == 0) {
        printf("Maximum subarray sum: %d\\n", result);
    }
    printf("\\n");
    
    // Test case 5: Large numbers
    int test5[] = {-1, -2, 5, -3, 4, 6, -2};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: ");
    printArray(test5, size5);
    if (maxSubarraySum(test5, size5, &result) == 0) {
        printf("Maximum subarray sum: %d\\n", result);
    }
    
    return 0;
}
