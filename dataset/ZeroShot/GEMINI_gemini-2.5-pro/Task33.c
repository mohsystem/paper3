#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Calculates the total time required for all customers to check out.
 * 
 * @param customers An array of positive integers representing the time each customer needs.
 * @param customers_len The number of customers in the array.
 * @param n The number of checkout tills.
 * @return The total time required.
 */
long long queueTime(const int* customers, int customers_len, int n) {
    if (customers_len == 0) {
        return 0;
    }

    // Use calloc to allocate and zero-initialize the tills array.
    // Use long long to prevent potential integer overflow.
    long long* tills = (long long*)calloc(n, sizeof(long long));
    if (tills == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    for (int i = 0; i < customers_len; i++) {
        // Find the till that will be free the soonest
        int min_till_index = 0;
        for (int j = 1; j < n; j++) {
            if (tills[j] < tills[min_till_index]) {
                min_till_index = j;
            }
        }
        // Assign the next customer to this till
        tills[min_till_index] += customers[i];
    }

    // The total time is the time the last till becomes free
    long long max_time = 0;
    for (int i = 0; i < n; i++) {
        if (tills[i] > max_time) {
            max_time = tills[i];
        }
    }

    free(tills); // Free the allocated memory
    return max_time;
}

int main() {
    // Test Case 1
    int customers1[] = {5, 3, 4};
    int len1 = sizeof(customers1) / sizeof(customers1[0]);
    printf("%lld\n", queueTime(customers1, len1, 1)); // Expected: 12

    // Test Case 2
    int customers2[] = {10, 2, 3, 3};
    int len2 = sizeof(customers2) / sizeof(customers2[0]);
    printf("%lld\n", queueTime(customers2, len2, 2)); // Expected: 10

    // Test Case 3
    int customers3[] = {2, 3, 10};
    int len3 = sizeof(customers3) / sizeof(customers3[0]);
    printf("%lld\n", queueTime(customers3, len3, 2)); // Expected: 12
    
    // Test Case 4 (empty queue)
    int customers4[] = {};
    int len4 = 0;
    printf("%lld\n", queueTime(customers4, len4, 1)); // Expected: 0

    // Test Case 5 (more tills than customers)
    int customers5[] = {1, 2, 3, 4, 5};
    int len5 = sizeof(customers5) / sizeof(customers5[0]);
    printf("%lld\n", queueTime(customers5, len5, 100)); // Expected: 5

    return 0;
}