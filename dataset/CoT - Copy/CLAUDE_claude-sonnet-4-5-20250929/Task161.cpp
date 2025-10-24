
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>

using namespace std;

int knapsack(int capacity, const vector<int>& weights, const vector<int>& values) {
    // Input validation
    if (capacity < 0) {
        throw invalid_argument("Capacity cannot be negative");
    }
    if (weights.size() != values.size()) {
        throw invalid_argument("Weights and values vectors must have the same length");
    }
    if (weights.empty()) {
        return 0;
    }
    
    // Validate individual weights and values
    for (size_t i = 0; i < weights.size(); i++) {
        if (weights[i] < 0 || values[i] < 0) {
            throw invalid_argument("Weights and values must be non-negative");
        }
    }
    
    int n = weights.size();
    
    // Prevent integer overflow for large capacity
    if (capacity > INT_MAX / 2) {
        throw invalid_argument("Capacity is too large");
    }
    
    // Dynamic programming approach
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));
    
    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            if (weights[i - 1] <= w) {
                dp[i][w] = max(
                    values[i - 1] + dp[i - 1][w - weights[i - 1]],
                    dp[i - 1][w]
                );
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }
    
    return dp[n][capacity];
}

int main() {
    try {
        // Test case 1: Basic case
        vector<int> weights1 = {2, 3, 4, 5};
        vector<int> values1 = {3, 4, 5, 6};
        int capacity1 = 5;
        cout << "Test 1 - Capacity: " << capacity1 << ", Max Value: " 
             << knapsack(capacity1, weights1, values1) << endl;
        
        // Test case 2: Single item
        vector<int> weights2 = {10};
        vector<int> values2 = {100};
        int capacity2 = 15;
        cout << "Test 2 - Capacity: " << capacity2 << ", Max Value: " 
             << knapsack(capacity2, weights2, values2) << endl;
        
        // Test case 3: No items fit
        vector<int> weights3 = {10, 20, 30};
        vector<int> values3 = {60, 100, 120};
        int capacity3 = 5;
        cout << "Test 3 - Capacity: " << capacity3 << ", Max Value: " 
             << knapsack(capacity3, weights3, values3) << endl;
        
        // Test case 4: All items fit
        vector<int> weights4 = {1, 2, 3};
        vector<int> values4 = {10, 20, 30};
        int capacity4 = 10;
        cout << "Test 4 - Capacity: " << capacity4 << ", Max Value: " 
             << knapsack(capacity4, weights4, values4) << endl;
        
        // Test case 5: Zero capacity
        vector<int> weights5 = {5, 10, 15};
        vector<int> values5 = {50, 100, 150};
        int capacity5 = 0;
        cout << "Test 5 - Capacity: " << capacity5 << ", Max Value: " 
             << knapsack(capacity5, weights5, values5) << endl;
             
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    
    return 0;
}
