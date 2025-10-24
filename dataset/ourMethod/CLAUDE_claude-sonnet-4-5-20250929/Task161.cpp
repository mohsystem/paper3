
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>

// Security: Validate input ranges to prevent integer overflow and excessive memory allocation
const int MAX_CAPACITY = 100000;
const int MAX_ITEMS = 10000;
const int MAX_WEIGHT = 100000;
const int MAX_VALUE = 1000000;

// Knapsack solver using dynamic programming
// Returns -1 on error, otherwise returns maximum value
int knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    // Security: Validate capacity bounds to prevent excessive memory allocation
    if (capacity < 0 || capacity > MAX_CAPACITY) {
        std::cerr << "Error: Capacity out of valid range [0, " << MAX_CAPACITY << "]" << std::endl;
        return -1;
    }
    
    // Security: Validate input vectors are not empty and have matching sizes
    if (weights.empty() || values.empty()) {
        std::cerr << "Error: Empty input vectors" << std::endl;
        return -1;
    }
    
    if (weights.size() != values.size()) {
        std::cerr << "Error: Weights and values vectors must have same size" << std::endl;
        return -1;
    }
    
    // Security: Validate number of items to prevent excessive memory allocation
    size_t n = weights.size();
    if (n > MAX_ITEMS) {
        std::cerr << "Error: Too many items (max " << MAX_ITEMS << ")" << std::endl;
        return -1;
    }
    
    // Security: Validate all weights and values are within acceptable ranges
    for (size_t i = 0; i < n; ++i) {
        if (weights[i] < 0 || weights[i] > MAX_WEIGHT) {
            std::cerr << "Error: Weight at index " << i << " out of range [0, " << MAX_WEIGHT << "]" << std::endl;
            return -1;
        }
        if (values[i] < 0 || values[i] > MAX_VALUE) {
            std::cerr << "Error: Value at index " << i << " out of range [0, " << MAX_VALUE << "]" << std::endl;
            return -1;
        }
    }
    
    // Security: Check for potential overflow before allocation
    // capacity + 1 cannot overflow since capacity <= MAX_CAPACITY
    size_t dpSize = static_cast<size_t>(capacity) + 1;
    
    // Use vector for automatic memory management (RAII)
    std::vector<int> dp(dpSize, 0);
    
    // Dynamic programming solution
    for (size_t i = 0; i < n; ++i) {
        // Iterate backwards to avoid using updated values in same iteration
        for (int w = capacity; w >= weights[i]; --w) {
            // Security: Check for potential integer overflow before addition
            if (dp[w - weights[i]] > std::numeric_limits<int>::max() - values[i]) {
                std::cerr << "Error: Integer overflow detected in value calculation" << std::endl;
                return -1;
            }
            dp[w] = std::max(dp[w], dp[w - weights[i]] + values[i]);
        }
    }
    
    return dp[capacity];
}

int main() {
    // Test case 1: Basic example
    {
        int capacity = 50;
        std::vector<int> weights = {10, 20, 30};
        std::vector<int> values = {60, 100, 120};
        int result = knapsack(capacity, weights, values);
        std::cout << "Test 1 - Capacity: " << capacity << ", Result: " << result << std::endl;
    }
    
    // Test case 2: Single item
    {
        int capacity = 10;
        std::vector<int> weights = {5};
        std::vector<int> values = {10};
        int result = knapsack(capacity, weights, values);
        std::cout << "Test 2 - Capacity: " << capacity << ", Result: " << result << std::endl;
    }
    
    // Test case 3: Items heavier than capacity
    {
        int capacity = 5;
        std::vector<int> weights = {10, 20, 30};
        std::vector<int> values = {60, 100, 120};
        int result = knapsack(capacity, weights, values);
        std::cout << "Test 3 - Capacity: " << capacity << ", Result: " << result << std::endl;
    }
    
    // Test case 4: Multiple items fit
    {
        int capacity = 100;
        std::vector<int> weights = {10, 20, 30, 40};
        std::vector<int> values = {10, 20, 30, 40};
        int result = knapsack(capacity, weights, values);
        std::cout << "Test 4 - Capacity: " << capacity << ", Result: " << result << std::endl;
    }
    
    // Test case 5: Invalid input (negative capacity)
    {
        int capacity = -10;
        std::vector<int> weights = {10, 20};
        std::vector<int> values = {60, 100};
        int result = knapsack(capacity, weights, values);
        std::cout << "Test 5 - Invalid capacity: " << capacity << ", Result: " << result << std::endl;
    }
    
    return 0;
}
