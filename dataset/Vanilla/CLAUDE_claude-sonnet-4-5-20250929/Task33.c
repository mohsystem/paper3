
#include <stdio.h>
#include <stdlib.h>

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int queueTime(int* customers, int customersSize, int n) {
    if (customers == NULL || customersSize == 0 || n <= 0) {
        return 0;
    }
    
    if (n >= customersSize) {
        int max = customers[0];
        for (int i = 1; i < customersSize; i++) {
            if (customers[i] > max) {
                max = customers[i];
            }
        }
        return max;
    }
    
    // Array to track till finish times
    int* tills = (int*)calloc(n, sizeof(int));
    
    // Process each customer
    for (int i = 0; i < customersSize; i++) {
        // Find the till with minimum time
        int minIdx = 0;
        for (int j = 1; j < n; j++) {
            if (tills[j] < tills[minIdx]) {
                minIdx = j;
            }
        }
        // Add customer time to that till
        tills[minIdx] += customers[i];
    }
    
    // Find the maximum time (last till to finish)
    int maxTime = tills[0];
    for (int i = 1; i < n; i++) {
        if (tills[i] > maxTime) {
            maxTime = tills[i];
        }
    }
    
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
    int test4[] = {1, 2, 3, 4, 5};
    printf("Test 4: %d (Expected: 6)\\n", queueTime(test4, 5, 3));
    
    // Test case 5
    int test5[] = {};
    printf("Test 5: %d (Expected: 0)\\n", queueTime(test5, 0, 1));
    
    return 0;
}
