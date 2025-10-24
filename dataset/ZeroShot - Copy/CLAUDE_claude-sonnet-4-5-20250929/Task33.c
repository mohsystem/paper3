
#include <stdio.h>
#include <stdlib.h>

// Helper function to find minimum value in array
int findMin(int* arr, int size) {
    int min = arr[0];
    int minIdx = 0;
    for (int i = 1; i < size; i++) {
        if (arr[i] < min) {
            min = arr[i];
            minIdx = i;
        }
    }
    return minIdx;
}

// Helper function to find maximum value in array
int findMax(int* arr, int size) {
    int max = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

int queueTime(int* customers, int customersSize, int n) {
    if (customers == NULL || customersSize == 0 || n <= 0) {
        return 0;
    }
    
    if (n >= customersSize) {
        return findMax(customers, customersSize);
    }
    
    // Create array to track finish time of each till
    int* tills = (int*)calloc(n, sizeof(int));
    
    // Process each customer
    for (int i = 0; i < customersSize; i++) {
        // Find the till that will be free first
        int minIdx = findMin(tills, n);
        // Add the customer to that till
        tills[minIdx] += customers[i];
    }
    
    // The total time is when the last till finishes
    int maxTime = findMax(tills, n);
    
    free(tills);
    return maxTime;
}

int main() {
    // Test case 1
    int test1[] = {5, 3, 4};
    printf("Test 1: %d (Expected: 12)\\n", queueTime(test1, 3, 1));
    
    // Test case 2
    int test2[] = {10, 2, 3, 3};
    printf("Test 2: %d (Expected: 10)\\n", queueTime(test2, 4, 2));
    
    // Test case 3
    int test3[] = {2, 3, 10};
    printf("Test 3: %d (Expected: 12)\\n", queueTime(test3, 3, 2));
    
    // Test case 4
    int test4[] = {};
    printf("Test 4: %d (Expected: 0)\\n", queueTime(test4, 0, 1));
    
    // Test case 5
    int test5[] = {1, 2, 3, 4, 5};
    printf("Test 5: %d (Expected: 5)\\n", queueTime(test5, 5, 100));
    
    return 0;
}
