#include <iostream>
#include <vector>
#include <algorithm>

/**
 * Solves the 0/1 knapsack problem using dynamic programming.
 *
 * @param capacity The maximum weight capacity of the knapsack.
 * @param weights  A vector of weights of the items.
 * @param values   A vector of values of the items.
 * @return The maximum value that can be put into the knapsack.
 */
int knapSack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    int n = weights.size();
    // Basic input validation
    if (n == 0 || capacity <= 0 || values.size() != n) {
        return 0;
    }

    // dp[w] will be storing the maximum value that can be obtained with knapsack capacity w.
    std::vector<int> dp(capacity + 1, 0);

    // Traverse through all items
    for (int i = 0; i < n; ++i) {
        // Traverse dp array from right to left to prevent using the same item multiple times
        for (int w = capacity; w >= weights[i]; --w) {
            dp[w] = std::max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }

    return dp[capacity];
}

int main() {
    // Test Case 1: Standard case
    std::vector<int> values1 = {60, 100, 120};
    std::vector<int> weights1 = {10, 20, 30};
    int capacity1 = 50;
    std::cout << "Test Case 1: Maximum value is " << knapSack(capacity1, weights1, values1) << std::endl;

    // Test Case 2: Small capacity with more items
    std::vector<int> values2 = {10, 40, 30, 50};
    std::vector<int> weights2 = {5, 4, 6, 3};
    int capacity2 = 10;
    std::cout << "Test Case 2: Maximum value is " << knapSack(capacity2, weights2, values2) << std::endl;

    // Test Case 3: All items fit
    std::vector<int> values3 = {60, 100, 120};
    std::vector<int> weights3 = {10, 20, 30};
    int capacity3 = 100;
    std::cout << "Test Case 3: Maximum value is " << knapSack(capacity3, weights3, values3) << std::endl;

    // Test Case 4: No items
    std::vector<int> values4 = {};
    std::vector<int> weights4 = {};
    int capacity4 = 50;
    std::cout << "Test Case 4: Maximum value is " << knapSack(capacity4, weights4, values4) << std::endl;

    // Test Case 5: Zero capacity
    std::vector<int> values5 = {60, 100, 120};
    std::vector<int> weights5 = {10, 20, 30};
    int capacity5 = 0;
    std::cout << "Test Case 5: Maximum value is " << knapSack(capacity5, weights5, values5) << std::endl;

    return 0;
}