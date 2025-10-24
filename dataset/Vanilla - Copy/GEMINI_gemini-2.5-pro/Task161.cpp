#include <iostream>
#include <vector>
#include <algorithm>

int knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    int n = values.size();
    if (n == 0) {
        return 0;
    }

    std::vector<int> dp(capacity + 1, 0);

    for (int i = 0; i < n; ++i) {
        for (int w = capacity; w >= weights[i]; --w) {
            dp[w] = std::max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }

    return dp[capacity];
}

int main() {
    // Test Case 1
    std::vector<int> values1 = {60, 100, 120};
    std::vector<int> weights1 = {10, 20, 30};
    int capacity1 = 50;
    std::cout << "Test Case 1: Maximum value = " << knapsack(capacity1, weights1, values1) << std::endl;

    // Test Case 2
    std::vector<int> values2 = {10, 20, 30};
    std::vector<int> weights2 = {5, 10, 15};
    int capacity2 = 50;
    std::cout << "Test Case 2: Maximum value = " << knapsack(capacity2, weights2, values2) << std::endl;

    // Test Case 3
    std::vector<int> values3 = {10, 40, 30, 50};
    std::vector<int> weights3 = {5, 4, 6, 3};
    int capacity3 = 10;
    std::cout << "Test Case 3: Maximum value = " << knapsack(capacity3, weights3, values3) << std::endl;

    // Test Case 4
    std::vector<int> values4 = {};
    std::vector<int> weights4 = {};
    int capacity4 = 10;
    std::cout << "Test Case 4: Maximum value = " << knapsack(capacity4, weights4, values4) << std::endl;

    // Test Case 5
    std::vector<int> values5 = {60, 100, 120};
    std::vector<int> weights5 = {10, 20, 30};
    int capacity5 = 0;
    std::cout << "Test Case 5: Maximum value = " << knapsack(capacity5, weights5, values5) << std::endl;

    return 0;
}