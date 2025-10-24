#include <stdio.h>
#include <stdlib.h>

int queueTime(const int *customers, int customers_length, int n) {
    if (customers_length == 0) {
        return 0;
    }

    // calloc allocates memory and initializes it to zero.
    int *tills = (int *)calloc(n, sizeof(int));
    if (tills == NULL) {
        // In case of memory allocation failure.
        return -1; 
    }

    for (int i = 0; i < customers_length; i++) {
        // Find the till that will be free earliest
        int minTillIndex = 0;
        for (int j = 1; j < n; j++) {
            if (tills[j] < tills[minTillIndex]) {
                minTillIndex = j;
            }
        }
        // Assign the customer to this till
        tills[minTillIndex] += customers[i];
    }

    // The total time is the time the last customer finishes
    int maxTime = 0;
    for (int i = 0; i < n; i++) {
        if (tills[i] > maxTime) {
            maxTime = tills[i];
        }
    }
    
    free(tills);
    return maxTime;
}

int main() {
    // Test Case 1
    int test1[] = {5, 3, 4};
    printf("%d\n", queueTime(test1, 3, 1));

    // Test Case 2
    int test2[] = {10, 2, 3, 3};
    printf("%d\n", queueTime(test2, 4, 2));

    // Test Case 3
    int test3[] = {2, 3, 10};
    printf("%d\n", queueTime(test3, 3, 2));

    // Test Case 4 (empty array)
    printf("%d\n", queueTime(NULL, 0, 1));

    // Test Case 5
    int test5[] = {1, 2, 3, 4, 5};
    printf("%d\n", queueTime(test5, 5, 100));

    return 0;
}