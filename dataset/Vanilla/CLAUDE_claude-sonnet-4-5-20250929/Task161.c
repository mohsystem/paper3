
#include <stdio.h>
#include <stdlib.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int knapsack(int capacity, int weights[], int values[], int n) {
    int** dp = (int**)malloc((n + 1) * sizeof(int*));
    for (int i = 0; i <= n; i++) {
        dp[i] = (int*)malloc((capacity + 1) * sizeof(int));
    }
    
    for (int i = 0; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            if (i == 0 || w == 0) {
                dp[i][w] = 0;
            } else if (weights[i - 1] <= w) {
                dp[i][w] = max(values[i - 1] + dp[i - 1][w - weights[i - 1]], 
                              dp[i - 1][w]);
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
    int n1 = 3;
    printf("Test 1 - Capacity: %d, Max Value: %d\\n", 
           capacity1, knapsack(capacity1, weights1, values1, n1));
    
    // Test case 2
    int capacity2 = 10;
    int weights2[] = {5, 4, 6, 3};
    int values2[] = {10, 40, 30, 50};
    int n2 = 4;
    printf("Test 2 - Capacity: %d, Max Value: %d\\n", 
           capacity2, knapsack(capacity2, weights2, values2, n2));
    
    // Test case 3
    int capacity3 = 7;
    int weights3[] = {1, 3, 4, 5};
    int values3[] = {1, 4, 5, 7};
    int n3 = 4;
    printf("Test 3 - Capacity: %d, Max Value: %d\\n", 
           capacity3, knapsack(capacity3, weights3, values3, n3));
    
    // Test case 4
    int capacity4 = 15;
    int weights4[] = {2, 3, 5, 7, 1};
    int values4[] = {10, 5, 15, 7, 6};
    int n4 = 5;
    printf("Test 4 - Capacity: %d, Max Value: %d\\n", 
           capacity4, knapsack(capacity4, weights4, values4, n4));
    
    // Test case 5
    int capacity5 = 0;
    int weights5[] = {1, 2, 3};
    int values5[] = {10, 20, 30};
    int n5 = 3;
    printf("Test 5 - Capacity: %d, Max Value: %d\\n", 
           capacity5, knapsack(capacity5, weights5, values5, n5));
    
    return 0;
}
