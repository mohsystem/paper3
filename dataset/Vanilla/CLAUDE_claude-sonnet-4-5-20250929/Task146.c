
#include <stdio.h>

int findMissingNumber(int arr[], int size, int n) {
    int expectedSum = n * (n + 1) / 2;
    int actualSum = 0;
    for (int i = 0; i < size; i++) {
        actualSum += arr[i];
    }
    return expectedSum - actualSum;
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 4, 5, 6};
    printf("Test 1 - Missing number: %d\\n", findMissingNumber(test1, 5, 6));
    
    // Test case 2
    int test2[] = {1, 2, 3, 4, 5, 6, 7, 9, 10};
    printf("Test 2 - Missing number: %d\\n", findMissingNumber(test2, 9, 10));
    
    // Test case 3
    int test3[] = {2, 3, 4, 5};
    printf("Test 3 - Missing number: %d\\n", findMissingNumber(test3, 4, 5));
    
    // Test case 4
    int test4[] = {1};
    printf("Test 4 - Missing number: %d\\n", findMissingNumber(test4, 1, 2));
    
    // Test case 5
    int test5[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16};
    printf("Test 5 - Missing number: %d\\n", findMissingNumber(test5, 15, 16));
    
    return 0;
}
