
#include <stdio.h>
#include <stdlib.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int knapsack(int capacity, int weights[], int values[], int n) {
    if (capacity <= 0 || weights == NULL || values == NULL || n <= 0) {
        return 0;
    }
    
    int** dp = (int**)malloc((n + 1) * sizeof(int*));
    for (int i = 0; i <= n; i++) {
        dp[i] = (int*)calloc(capacity + 1, sizeof(int));
    }
    
    for (int i = 1; i <= n; i++) {
        for (int w = 1; w <= capacity; w++) {
            if (weights[i - 1] <= w) {
                dp[i][w] = max(dp[i - 1][w], 
                              values[i - 1] + dp[i - 1][w - weights[i - 1]]);
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }
    
    int result = dp[n][capacity];
    
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);
    
    return result;
}

int main() {
    // Test case 1
    int capacity1 = 50;
    int weights1[] = {10, 20, 30};
    int values1[] = {60, 100, 120};
    printf("Test 1 - Capacity: %d, Max Value: %d\\n", 
           capacity1, knapsack(capacity1, weights1, values1, 3));
    
    // Test case 2
    int capacity2 = 10;
    int weights2[] = {5, 4, 6, 3};
    int values2[] = {10, 40, 30, 50};
    printf("Test 2 - Capacity: %d, Max Value: %d\\n", 
           capacity2, knapsack(capacity2, weights2, values2, 4));
    
    // Test case 3
    int capacity3 = 7;
    int weights3[] = {1, 3, 4, 5};
    int values3[] = {1, 4, 5, 7};
    printf("Test 3 - Capacity: %d, Max Value: %d\\n", 
           capacity3, knapsack(capacity3, weights3, values3, 4));
    
    // Test case 4
    int capacity4 = 15;
    int weights4[] = {2, 3, 5, 7, 1};
    int values4[] = {10, 5, 15, 7, 6};
    printf("Test 4 - Capacity: %d, Max Value: %d\\n", 
           capacity4, knapsack(capacity4, weights4, values4, 5));
    
    // Test case 5
    int capacity5 = 8;
    int weights5[] = {2, 3, 4, 5};
    int values5[] = {3, 4, 5, 6};
    printf("Test 5 - Capacity: %d, Max Value: %d\\n", 
           capacity5, knapsack(capacity5, weights5, values5, 4));
    
    return 0;
}
