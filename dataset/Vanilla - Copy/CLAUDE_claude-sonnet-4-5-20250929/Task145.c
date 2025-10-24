
#include <stdio.h>
#include <limits.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int maxSubarraySum(int arr[], int size) {
    if (size == 0) {
        return 0;
    }
    
    int maxSum = arr[0];
    int currentSum = arr[0];
    
    for (int i = 1; i < size; i++) {
        currentSum = max(arr[i], currentSum + arr[i]);
        maxSum = max(maxSum, currentSum);
    }
    
    return maxSum;
}

int main() {
    // Test case 1: Mixed positive and negative numbers
    int test1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    printf("Test 1: %d\\n", maxSubarraySum(test1, 9)); // Expected: 6
    
    // Test case 2: All negative numbers
    int test2[] = {-5, -2, -8, -1, -4};
    printf("Test 2: %d\\n", maxSubarraySum(test2, 5)); // Expected: -1
    
    // Test case 3: All positive numbers
    int test3[] = {1, 2, 3, 4, 5};
    printf("Test 3: %d\\n", maxSubarraySum(test3, 5)); // Expected: 15
    
    // Test case 4: Single element
    int test4[] = {5};
    printf("Test 4: %d\\n", maxSubarraySum(test4, 1)); // Expected: 5
    
    // Test case 5: Mix with zeros
    int test5[] = {-2, 0, -1, 3, -1, 2, -3, 4};
    printf("Test 5: %d\\n", maxSubarraySum(test5, 8)); // Expected: 5
    
    return 0;
}
