#include <stdio.h>
#include <stdlib.h>

/**
 * Calculates the total time required for all customers to check out.
 *
 * @param customers An array of positive integers representing the queue.
 * @param customers_size The size of the customers array.
 * @param n The number of checkout tills.
 * @return The total time required.
 */
int queueTime(const int* customers, int customers_size, int n) {
    if (customers_size == 0) {
        return 0;
    }

    int* tills = (int*)calloc(n, sizeof(int));
    if (tills == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    for (int i = 0; i < customers_size; i++) {
        // Find the till that will be free the soonest
        int min_index = 0;
        for (int j = 1; j < n; j++) {
            if (tills[j] < tills[min_index]) {
                min_index = j;
            }
        }
        // Assign the customer to this till
        tills[min_index] += customers[i];
    }

    // The total time is the time the last till becomes free
    int max_time = 0;
    for (int i = 0; i < n; i++) {
        if (tills[i] > max_time) {
            max_time = tills[i];
        }
    }

    free(tills);
    return max_time;
}

int main() {
    // Test cases
    int c1[] = {};
    printf("%d\n", queueTime(c1, 0, 1)); // Expected: 0

    int c2[] = {5, 3, 4};
    printf("%d\n", queueTime(c2, sizeof(c2) / sizeof(c2[0]), 1)); // Expected: 12

    int c3[] = {10, 2, 3, 3};
    printf("%d\n", queueTime(c3, sizeof(c3) / sizeof(c3[0]), 2)); // Expected: 10

    int c4[] = {2, 3, 10};
    printf("%d\n", queueTime(c4, sizeof(c4) / sizeof(c4[0]), 2)); // Expected: 12

    int c5[] = {1, 2, 3, 4, 5};
    printf("%d\n", queueTime(c5, sizeof(c5) / sizeof(c5[0]), 100)); // Expected: 5

    return 0;
}