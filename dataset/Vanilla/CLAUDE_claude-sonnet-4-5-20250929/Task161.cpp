
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int knapsack(int capacity, vector<int>& weights, vector<int>& values, int n) {
    vector<vector<int>> dp(n + 1, vector<int>(capacity + 1, 0));
    
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
    return dp[n][capacity];
}

int main() {
    // Test case 1
    int capacity1 = 50;
    vector<int> weights1 = {10, 20, 30};
    vector<int> values1 = {60, 100, 120};
    cout << "Test 1 - Capacity: " << capacity1 << ", Max Value: " 
         << knapsack(capacity1, weights1, values1, weights1.size()) << endl;
    
    // Test case 2
    int capacity2 = 10;
    vector<int> weights2 = {5, 4, 6, 3};
    vector<int> values2 = {10, 40, 30, 50};
    cout << "Test 2 - Capacity: " << capacity2 << ", Max Value: " 
         << knapsack(capacity2, weights2, values2, weights2.size()) << endl;
    
    // Test case 3
    int capacity3 = 7;
    vector<int> weights3 = {1, 3, 4, 5};
    vector<int> values3 = {1, 4, 5, 7};
    cout << "Test 3 - Capacity: " << capacity3 << ", Max Value: " 
         << knapsack(capacity3, weights3, values3, weights3.size()) << endl;
    
    // Test case 4
    int capacity4 = 15;
    vector<int> weights4 = {2, 3, 5, 7, 1};
    vector<int> values4 = {10, 5, 15, 7, 6};
    cout << "Test 4 - Capacity: " << capacity4 << ", Max Value: " 
         << knapsack(capacity4, weights4, values4, weights4.size()) << endl;
    
    // Test case 5
    int capacity5 = 0;
    vector<int> weights5 = {1, 2, 3};
    vector<int> values5 = {10, 20, 30};
    cout << "Test 5 - Capacity: " << capacity5 << ", Max Value: " 
         << knapsack(capacity5, weights5, values5, weights5.size()) << endl;
    
    return 0;
}
