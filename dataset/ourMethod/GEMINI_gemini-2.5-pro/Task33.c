#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/**
 * @brief Calculates the total time required for all customers to check out.
 *
 * @param customers An array of positive integers representing the time each customer needs.
 * @param customers_size The number of customers in the queue.
 * @param n The number of checkout tills.
 * @return The total time required.
 */
int queueTime(const int* customers, int customers_size, int n) {
    if (customers_size == 0) {
        return 0;
    }

    // Allocate memory for tills and initialize to zero.
    // calloc is used to ensure all elements are initialized to 0.
    int* tills = (int*)calloc(n, sizeof(int));
    if (tills == NULL) {
        // In a real-world scenario, proper error handling is crucial.
        // For this problem, we assume valid inputs and successful allocation.
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return -1; // Return an error code
    }

    // Iterate through each customer in the queue.
    for (int i = 0; i < customers_size; ++i) {
        // Find the index of the till that will become free the soonest.
        int min_time_idx = 0;
        for (int j = 1; j < n; ++j) {
            if (tills[j] < tills[min_time_idx]) {
                min_time_idx = j;
            }
        }
        
        // Assign the current customer to this till by adding their time.
        tills[min_time_idx] += customers[i];
    }

    // The total time is determined by the till that finishes last.
    // Find the maximum time among all tills.
    int max_time = 0;
    for (int i = 0; i < n; ++i) {
        if (tills[i] > max_time) {
            max_time = tills[i];
        }
    }

    // Free the dynamically allocated memory.
    free(tills);
    tills = NULL; // Prevent dangling pointer.

    return max_time;
}

int main() {
    // Test Case 1
    int c1[] = {5, 3, 4};
    int s1 = sizeof(c1) / sizeof(c1[0]);
    printf("Test 1: queueTime({5, 3, 4}, 1) -> %d (Expected: 12)\n", queueTime(c1, s1, 1));

    // Test Case 2
    int c2[] = {10, 2, 3, 3};
    int s2 = sizeof(c2) / sizeof(c2[0]);
    printf("Test 2: queueTime({10, 2, 3, 3}, 2) -> %d (Expected: 10)\n", queueTime(c2, s2, 2));

    // Test Case 3
    int c3[] = {2, 3, 10};
    int s3 = sizeof(c3) / sizeof(c3[0]);
    printf("Test 3: queueTime({2, 3, 10}, 2) -> %d (Expected: 12)\n", queueTime(c3, s3, 2));

    // Test Case 4 (empty queue)
    int* c4 = NULL;
    int s4 = 0;
    printf("Test 4: queueTime({}, 1) -> %d (Expected: 0)\n", queueTime(c4, s4, 1));
    
    // Test Case 5 (more tills than customers)
    int c5[] = {1, 2, 3, 4, 5};
    int s5 = sizeof(c5) / sizeof(c5[0]);
    printf("Test 5: queueTime({1, 2, 3, 4, 5}, 100) -> %d (Expected: 5)\n", queueTime(c5, s5, 100));

    return 0;
}