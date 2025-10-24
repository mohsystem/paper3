#include <bits/stdc++.h>
using namespace std;

int knapsack(int capacity, const vector<int>& weights, const vector<int>& values) {
    if (capacity <= 0 || weights.size() != values.size() || weights.empty()) return 0;
    vector<int> dp(capacity + 1, 0);
    for (size_t i = 0; i < weights.size(); ++i) {
        int w = weights[i];
        int v = values[i];
        for (int c = capacity; c >= w; --c) {
            dp[c] = max(dp[c], dp[c - w] + v);
        }
    }
    return dp[capacity];
}

int main() {
    // Test case 1
    int cap1 = 5;
    vector<int> w1 = {1, 2, 3};
    vector<int> v1 = {6, 10, 12};
    cout << knapsack(cap1, w1, v1) << endl; // Expected 22

    // Test case 2
    int cap2 = 3;
    vector<int> w2 = {4, 5};
    vector<int> v2 = {7, 8};
    cout << knapsack(cap2, w2, v2) << endl; // Expected 0

    // Test case 3
    int cap3 = 0;
    vector<int> w3 = {1, 2};
    vector<int> v3 = {10, 20};
    cout << knapsack(cap3, w3, v3) << endl; // Expected 0

    // Test case 4
    int cap4 = 4;
    vector<int> w4 = {4};
    vector<int> v4 = {10};
    cout << knapsack(cap4, w4, v4) << endl; // Expected 10

    // Test case 5
    int cap5 = 10;
    vector<int> w5 = {2, 5, 1, 3, 4};
    vector<int> v5 = {3, 4, 7, 8, 9};
    cout << knapsack(cap5, w5, v5) << endl; // Expected 27

    return 0;
}