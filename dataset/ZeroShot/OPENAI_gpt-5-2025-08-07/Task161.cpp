#include <bits/stdc++.h>
using namespace std;

static const int MAX_CAPACITY = 2'000'000;

long long maxKnapsackValue(int capacity, const vector<int>& weights, const vector<int>& values) {
    if (capacity < 0) return 0LL;
    if (weights.size() != values.size()) return -1LL;
    if (capacity > MAX_CAPACITY) return -1LL;
    size_t n = weights.size();
    for (size_t i = 0; i < n; ++i) {
        if (weights[i] < 0 || values[i] < 0) return -1LL;
    }
    vector<long long> dp(static_cast<size_t>(capacity) + 1, 0LL);
    for (size_t i = 0; i < n; ++i) {
        int wt = weights[i];
        long long val = values[i];
        if (wt <= capacity) {
            for (int c = capacity; c >= wt; --c) {
                long long base = dp[c - wt];
                long long candidate;
                if (base > LLONG_MAX - val) candidate = LLONG_MAX;
                else candidate = base + val;
                if (candidate > dp[c]) dp[c] = candidate;
            }
        }
    }
    return dp[capacity];
}

int main() {
    // Test case 1
    int cap1 = 50;
    vector<int> w1 = {10, 20, 30};
    vector<int> v1 = {60, 100, 120};
    cout << maxKnapsackValue(cap1, w1, v1) << "\n"; // Expected 220

    // Test case 2
    int cap2 = 7;
    vector<int> w2 = {1, 3, 4, 5};
    vector<int> v2 = {1, 4, 5, 7};
    cout << maxKnapsackValue(cap2, w2, v2) << "\n"; // Expected 9

    // Test case 3
    int cap3 = 0;
    vector<int> w3 = {1, 2, 3};
    vector<int> v3 = {10, 15, 40};
    cout << maxKnapsackValue(cap3, w3, v3) << "\n"; // Expected 0

    // Test case 4
    int cap4 = 10;
    vector<int> w4 = {3, 4, 5, 9, 4};
    vector<int> v4 = {4, 5, 7, 10, 6};
    cout << maxKnapsackValue(cap4, w4, v4) << "\n"; // Expected 13

    // Test case 5
    int cap5 = 15;
    vector<int> w5 = {12, 2, 1, 1, 4};
    vector<int> v5 = {4, 2, 2, 1, 10};
    cout << maxKnapsackValue(cap5, w5, v5) << "\n"; // Expected 15

    return 0;
}