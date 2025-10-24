
#include <stdio.h>
#include <stdlib.h>

int queueTime(int* customers, int customersSize, int n) {
    // Input validation
    if (customers == NULL || customersSize == 0 || n <= 0) {
        return 0;
    }
    
    // If number of tills is greater than or equal to customers,
    // return the maximum time among all customers
    if (n >= customersSize) {
        int max = customers[0];
        for (int i = 1; i < customersSize; i++) {
            if (customers[i] > max) {
                max = customers[i];
            }
        }
        return max;
    }
    
    // Create an array to represent the tills and their busy time
    int* tills = (int*)calloc(n, sizeof(int));
    if (tills == NULL) {
        return 0;
    }
    
    // Assign each customer to the till that will be free first
    for (int i = 0; i < customersSize; i++) {
        // Find the till with minimum time
        int minIndex = 0;
        int minTime = tills[0];
        for (int j = 1; j < n; j++) {
            if (tills[j] < minTime) {
                minTime = tills[j];
                minIndex = j;
            }
        }
        // Assign customer to the till with minimum time
        tills[minIndex] += customers[i];
    }
    
    // Find the maximum time among all tills
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
    int customers1[] = {5, 3, 4};
    printf("Test 1: %d\\n", queueTime(customers1, 3, 1)); // Expected: 12
    
    // Test case 2
    int customers2[] = {10, 2, 3, 3};
    printf("Test 2: %d\\n", queueTime(customers2, 4, 2)); // Expected: 10
    
    // Test case 3
    int customers3[] = {2, 3, 10};
    printf("Test 3: %d\\n", queueTime(customers3, 3, 2)); // Expected: 12
    
    // Test case 4
    printf("Test 4: %d\\n", queueTime(NULL, 0, 1)); // Expected: 0
    
    // Test case 5
    int customers5[] = {1, 2, 3, 4, 5};
    printf("Test 5: %d\\n", queueTime(customers5, 5, 5)); // Expected: 5
    
    return 0;
}
