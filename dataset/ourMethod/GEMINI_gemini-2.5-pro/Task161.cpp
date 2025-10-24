#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

/**
 * Solves the 0/1 knapsack problem.
 *
 * @param capacity The maximum weight capacity of the knapsack.
 * @param weights  A vector of weights of the items.
 * @param values   A vector of values of the items.
 * @return The maximum value that can be put into the knapsack, or -1 if input is invalid.
 */
long long knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    // Input validation
    if (capacity < 0 || weights.size() != values.size()) {
        return -1;
    }
    for (int w : weights) {
        if (w < 0) return -1;
    }
    for (int v : values) {
        if (v < 0) return -1;
    }

    size_t n = weights.size();
    if (n == 0 || capacity == 0) {
        return 0;
    }

    // DP table to store results of subproblems. Using long long to prevent overflow.
    // dp[i][w] will be the maximum value that can be obtained with a capacity w using the first i items.
    std::vector<std::vector<long long>> dp(n + 1, std::vector<long long>(capacity + 1, 0));

    // Build table dp[][] in bottom up manner
    for (size_t i = 1; i <= n; ++i) {
        for (int w = 1; w <= capacity; ++w) {
            // Current item's weight and value (i-1 is the index in original vectors)
            int currentWeight = weights[i - 1];
            int currentValue = values[i - 1];

            if (currentWeight <= w) {
                // Two choices:
                // 1. Include the current item
                // 2. Exclude the current item
                dp[i][w] = std::max((long long)currentValue + dp[i - 1][w - currentWeight], dp[i - 1][w]);
            } else {
                // Cannot include the current item
                dp[i][w] = dp[i - 1][w];
            }
        }
    }

    return dp[n][capacity];
}

int main() {
    // Test Case 1: Standard case
    std::vector<int> weights1 = {10, 20, 30};
    std::vector<int> values1 = {60, 100, 120};
    int capacity1 = 50;
    std::cout << "Test Case 1: Max value = " << knapsack(capacity1, weights1, values1) << std::endl; // Expected: 220

    // Test Case 2: No items
    std::vector<int> weights2 = {};
    std::vector<int> values2 = {};
    int capacity2 = 10;
    std::cout << "Test Case 2: Max value = " << knapsack(capacity2, weights2, values2) << std::endl; // Expected: 0

    // Test Case 3: Capacity too small for any item
    std::vector<int> weights3 = {10, 20};
    std::vector<int> values3 = {100, 200};
    int capacity3 = 5;
    std::cout << "Test Case 3: Max value = " << knapsack(capacity3, weights3, values3) << std::endl; // Expected: 0

    // Test Case 4: All items fit
    std::vector<int> weights4 = {10, 20, 30};
    std::vector<int> values4 = {60, 100, 120};
    int capacity4 = 100;
    std::cout << "Test Case 4: Max value = " << knapsack(capacity4, weights4, values4) << std::endl; // Expected: 280

    // Test Case 5: Complex case
    std::vector<int> weights5 = {5, 4, 6, 3};
    std::vector<int> values5 = {10, 40, 30, 50};
    int capacity5 = 10;
    std::cout << "Test Case 5: Max value = " << knapsack(capacity5, weights5, values5) << std::endl; // Expected: 90

    return 0;
}