#include <iostream>
#include <vector>
#include <algorithm>

long long knapsack(int capacity, const std::vector<int>& weights, const std::vector<int>& values) {
    if (capacity <= 0 || weights.size() != values.size()) {
        return 0LL;
    }
    std::vector<long long> dp(static_cast<size_t>(capacity) + 1, 0LL);
    for (size_t i = 0; i < weights.size(); ++i) {
        int w = weights[i];
        int v = values[i];
        if (w <= 0 || v < 0) {
            continue; // Skip invalid items
        }
        for (int c = capacity; c >= w; --c) {
            long long cand = dp[c - w] + static_cast<long long>(v);
            if (cand > dp[c]) {
                dp[c] = cand;
            }
        }
    }
    return dp[capacity];
}

int main() {
    // Test case 1
    int cap1 = 50;
    std::vector<int> w1{10, 20, 30};
    std::vector<int> v1{60, 100, 120};
    std::cout << knapsack(cap1, w1, v1) << "\n"; // Expected: 220

    // Test case 2
    int cap2 = 0;
    std::vector<int> w2{5, 10};
    std::vector<int> v2{10, 20};
    std::cout << knapsack(cap2, w2, v2) << "\n"; // Expected: 0

    // Test case 3
    int cap3 = 7;
    std::vector<int> w3{1, 3, 4, 5};
    std::vector<int> v3{1, 4, 5, 7};
    std::cout << knapsack(cap3, w3, v3) << "\n"; // Expected: 9

    // Test case 4
    int cap4 = 10;
    std::vector<int> w4{6, 3, 4, 2};
    std::vector<int> v4{30, 14, 16, 9};
    std::cout << knapsack(cap4, w4, v4) << "\n"; // Expected: 46

    // Test case 5
    int cap5 = 5;
    std::vector<int> w5{4, 2, 3};
    std::vector<int> v5{10, 4, 7};
    std::cout << knapsack(cap5, w5, v5) << "\n"; // Expected: 11

    return 0;
}