
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Function to calculate total checkout time
// Security: All inputs validated, bounds checked, safe memory management
int queueTime(const int* customers, int customersSize, int n) {
    // Input validation: check for NULL pointer
    if (customers == NULL && customersSize > 0) {
        return 0;
    }
    
    // Input validation: check for negative or zero tills
    if (n <= 0) {
        return 0;
    }
    
    // Input validation: check for empty customer queue
    if (customersSize <= 0) {
        return 0;
    }
    
    // Input validation: ensure customersSize is reasonable to prevent excessive operations
    const int MAX_CUSTOMERS = 1000000;
    if (customersSize > MAX_CUSTOMERS) {
        return 0;
    }
    
    // Input validation: ensure n doesn't exceed reasonable bounds
    const int MAX_TILLS = 100000;
    if (n > MAX_TILLS) {
        n = MAX_TILLS;
    }
    
    // Input validation: validate customer times are non-negative
    for (int i = 0; i < customersSize; i++) {
        if (customers[i] < 0) {
            return 0; // Invalid input
        }
    }
    
    // Limit n to actual number of customers
    if (n > customersSize) {
        n = customersSize;
    }
    
    // Allocate memory for tills array with validation
    int* tills = (int*)calloc((size_t)n, sizeof(int));
    if (tills == NULL) {
        return 0; // Memory allocation failed
    }
    
    // All tills initialized to 0 by calloc
    
    // Process each customer in queue order
    for (int i = 0; i < customersSize; i++) {
        // Find the till with minimum time
        int minIndex = 0;
        int minTime = tills[0];
        
        // Bounds-checked loop to find minimum
        for (int j = 1; j < n; j++) {
            if (tills[j] < minTime) {
                minTime = tills[j];
                minIndex = j;
            }
        }
        
        // Check for integer overflow before addition
        if (tills[minIndex] > INT_MAX - customers[i]) {
            free(tills); // Clean up before returning
            return INT_MAX; // Overflow would occur
        }
        
        // Assign customer to the till with minimum time
        tills[minIndex] += customers[i];
    }
    
    // Find maximum time among all tills
    int maxTime = tills[0];
    for (int i = 1; i < n; i++) {
        if (tills[i] > maxTime) {
            maxTime = tills[i];
        }
    }
    
    // Free allocated memory
    free(tills);
    tills = NULL;
    
    return maxTime;
}

int main(void) {
    // Test case 1: Single till
    int test1[] = {5, 3, 4};
    printf("Test 1: %d (expected 12)\\n", queueTime(test1, 3, 1));
    
    // Test case 2: Two tills with varying times
    int test2[] = {10, 2, 3, 3};
    printf("Test 2: %d (expected 10)\\n", queueTime(test2, 4, 2));
    
    // Test case 3: Two tills
    int test3[] = {2, 3, 10};
    printf("Test 3: %d (expected 12)\\n", queueTime(test3, 3, 2));
    
    // Test case 4: Empty queue
    printf("Test 4: %d (expected 0)\\n", queueTime(NULL, 0, 2));
    
    // Test case 5: More tills than customers
    int test5[] = {1, 2, 3};
    printf("Test 5: %d (expected 3)\\n", queueTime(test5, 3, 5));
    
    return 0;
}
