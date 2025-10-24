
#include <stdio.h>

int findMissingNumber(const int* arr, int arrSize, int n) {
    // Calculate expected sum of 1 to n
    long long expectedSum = (long long)n * (n + 1) / 2;
    
    // Calculate actual sum of array elements
    long long actualSum = 0;
    for (int i = 0; i < arrSize; i++) {
        actualSum += arr[i];
    }
    
    // Missing number is the difference
    return expectedSum - actualSum;
}

int main() {
    // Test case 1: Missing number is 5
    int test1[] = {1, 2, 3, 4, 6, 7, 8, 9, 10};
    printf("Test 1 - Missing number: %d\\n", findMissingNumber(test1, 9, 10));
    
    // Test case 2: Missing number is 1
    int test2[] = {2, 3, 4, 5};
    printf("Test 2 - Missing number: %d\\n", findMissingNumber(test2, 4, 5));
    
    // Test case 3: Missing number is 10
    int test3[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    printf("Test 3 - Missing number: %d\\n", findMissingNumber(test3, 9, 10));
    
    // Test case 4: Missing number is 3
    int test4[] = {1, 2, 4, 5, 6};
    printf("Test 4 - Missing number: %d\\n", findMissingNumber(test4, 5, 6));
    
    // Test case 5: Missing number is 7
    int test5[] = {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12};
    printf("Test 5 - Missing number: %d\\n", findMissingNumber(test5, 11, 12));
    
    return 0;
}
