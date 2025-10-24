
#include <stdio.h>
#include <stdlib.h>

/**
 * Finds the missing number in an array containing unique integers from 1 to n
 * @param arr Array of integers with one missing number
 * @param size Size of the array
 * @return The missing number
 */
int findMissingNumber(const int* arr, int size) {
    if (arr == NULL || size == 0) {
        return 1;
    }
    
    int n = size + 1;
    // Using mathematical formula: sum of 1 to n = n*(n+1)/2
    long long expectedSum = (long long)n * (n + 1) / 2;
    long long actualSum = 0;
    
    for (int i = 0; i < size; i++) {
        actualSum += arr[i];
    }
    
    return (int)(expectedSum - actualSum);
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]");
}

int main() {
    // Test case 1: Missing number is 5
    int test1[] = {1, 2, 3, 4, 6};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1 - Array: ");
    printArray(test1, size1);
    printf("\\nMissing number: %d\\n\\n", findMissingNumber(test1, size1));
    
    // Test case 2: Missing number is 1
    int test2[] = {2, 3, 4, 5, 6};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2 - Array: ");
    printArray(test2, size2);
    printf("\\nMissing number: %d\\n\\n", findMissingNumber(test2, size2));
    
    // Test case 3: Missing number is 10
    int test3[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3 - Array: ");
    printArray(test3, size3);
    printf("\\nMissing number: %d\\n\\n", findMissingNumber(test3, size3));
    
    // Test case 4: Missing number is 3
    int test4[] = {1, 2, 4, 5, 6, 7};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4 - Array: ");
    printArray(test4, size4);
    printf("\\nMissing number: %d\\n\\n", findMissingNumber(test4, size4));
    
    // Test case 5: Array with single element missing (n=2)
    int test5[] = {2};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5 - Array: ");
    printArray(test5, size5);
    printf("\\nMissing number: %d\\n\\n", findMissingNumber(test5, size5));
    
    return 0;
}
