#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

/**
 * Solves the 0/1 knapsack problem using dynamic programming.
 *
 * @param capacity The maximum weight capacity of the knapsack.
 * @param weights  A vector of weights of the items.
 * @param values   A vector of values of the items.
 * @return The maximum value that can be put into the knapsack.
 */
int knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    // --- Secure Code: Input Validation ---
    if (capacity < 0 || weights.size() != values.size()) {
        // Invalid arguments, returning 0 as a safe default.
        return 0;
    }
    
    size_t n = values.size();
    if (n == 0 || capacity == 0) {
        return 0;
    }

    // dp[w] will be the maximum value that can be obtained for a knapsack of capacity w
    std::vector<int> dp(capacity + 1, 0);

    // Build table dp[] in a bottom-up manner
    for (size_t i = 0; i < n; ++i) {
        // --- Secure Code: Check for invalid item properties ---
        if (weights[i] < 0 || values[i] < 0) {
            // Skip items with negative weight or value.
            continue;
        }
        // Traverse backwards to ensure each item is considered only once (0/1 property)
        for (int w = capacity; w >= weights[i]; --w) {
            dp[w] = std::max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }

    return dp[capacity];
}

void run_test_cases() {
    // Test Case 1: General case
    int capacity1 = 50;
    std::vector<int> values1 = {60, 100, 120};
    std::vector<int> weights1 = {10, 20, 30};
    std::cout << "Test Case 1: Max value = " << knapsack(capacity1, weights1, values1) << std::endl;

    // Test Case 2: Empty items
    int capacity2 = 10;
    std::vector<int> values2 = {};
    std::vector<int> weights2 = {};
    std::cout << "Test Case 2: Max value = " << knapsack(capacity2, weights2, values2) << std::endl;

    // Test Case 3: Zero capacity
    int capacity3 = 0;
    std::vector<int> values3 = {60, 100};
    std::vector<int> weights3 = {10, 20};
    std::cout << "Test Case 3: Max value = " << knapsack(capacity3, weights3, values3) << std::endl;

    // Test Case 4: Items that partially fit
    int capacity4 = 5;
    std::vector<int> values4 = {3, 4, 5, 6};
    std::vector<int> weights4 = {2, 3, 4, 5};
    std::cout << "Test Case 4: Max value = " << knapsack(capacity4, weights4, values4) << std::endl;

    // Test Case 5: More complex case
    int capacity5 = 10;
    std::vector<int> values5 = {10, 40, 30, 50};
    std::vector<int> weights5 = {5, 4, 6, 3};
    std::cout << "Test Case 5: Max value = " << knapsack(capacity5, weights5, values5) << std::endl;
}

int main() {
    run_test_cases();
    return 0;
}