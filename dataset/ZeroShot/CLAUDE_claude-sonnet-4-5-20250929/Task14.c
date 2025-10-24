
#include <stdio.h>
#include <stdlib.h>

int findEqualSumIndex(int arr[], int length) {
    if (arr == NULL || length == 0) {
        return -1;
    }
    
    // Calculate total sum of array
    long long totalSum = 0;
    for (int i = 0; i < length; i++) {
        totalSum += arr[i];
    }
    
    // Iterate through array and check if left sum equals right sum
    long long leftSum = 0;
    for (int i = 0; i < length; i++) {
        // Right sum is total sum minus left sum minus current element
        long long rightSum = totalSum - leftSum - arr[i];
        
        if (leftSum == rightSum) {
            return i;
        }
        
        // Add current element to left sum for next iteration
        leftSum += arr[i];
    }
    
    return -1;
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 3, 4, 3, 2, 1};
    printf("Test 1: %d\\n", findEqualSumIndex(test1, 7)); // Expected: 3
    
    // Test case 2
    int test2[] = {1, 100, 50, -51, 1, 1};
    printf("Test 2: %d\\n", findEqualSumIndex(test2, 6)); // Expected: 1
    
    // Test case 3
    int test3[] = {20, 10, -80, 10, 10, 15, 35};
    printf("Test 3: %d\\n", findEqualSumIndex(test3, 7)); // Expected: 0
    
    // Test case 4
    int test4[] = {1, 2, 3};
    printf("Test 4: %d\\n", findEqualSumIndex(test4, 3)); // Expected: -1
    
    // Test case 5
    int test5[] = {10};
    printf("Test 5: %d\\n", findEqualSumIndex(test5, 1)); // Expected: 0
    
    return 0;
}
