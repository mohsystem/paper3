#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

int knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    if (capacity < 0) {
        throw std::invalid_argument("Capacity must be non-negative.");
    }
    if (weights.size() != values.size()) {
        throw std::invalid_argument("Weights and values must have the same length.");
    }
    for (size_t i = 0; i < weights.size(); ++i) {
        if (weights[i] < 0) {
            throw std::invalid_argument("Weights must be non-negative.");
        }
    }

    std::vector<int> dp(static_cast<size_t>(capacity) + 1, 0);
    for (size_t i = 0; i < weights.size(); ++i) {
        int wt = weights[i];
        int val = values[i];
        if (wt > capacity) {
            continue;
        }
        for (int w = capacity; w >= wt; --w) {
            int cand = dp[static_cast<size_t>(w - wt)] + val;
            if (cand > dp[static_cast<size_t>(w)]) {
                dp[static_cast<size_t>(w)] = cand;
            }
        }
    }
    return dp[static_cast<size_t>(capacity)];
}

void run_test(int test_num, int capacity, const std::vector<int>& weights, const std::vector<int>& values, int expected) {
    int result = knapsack(capacity, weights, values);
    std::cout << "Test " << test_num << " -> result=" << result << " expected=" << expected << "\n";
}

int main() {
    // Test 1: Classic example
    run_test(1, 50, {10, 20, 30}, {60, 100, 120}, 220);

    // Test 2: Empty items
    run_test(2, 10, {}, {}, 0);

    // Test 3: Zero capacity
    run_test(3, 0, {1, 2, 3}, {10, 20, 30}, 0);

    // Test 4: Items heavier than capacity
    run_test(4, 5, {6, 7, 8}, {10, 20, 30}, 0);

    // Test 5: Mixed items
    run_test(5, 7, {1, 3, 4, 5}, {1, 4, 5, 7}, 9);

    return 0;
}