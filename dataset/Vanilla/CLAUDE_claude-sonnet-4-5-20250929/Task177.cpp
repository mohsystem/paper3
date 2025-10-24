
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <climits>

using namespace std;

int constrainedSubsetSum(vector<int>& nums, int k) {
    int n = nums.size();
    vector<int> dp(n);
    deque<int> dq;
    
    int maxSum = INT_MIN;
    
    for (int i = 0; i < n; i++) {
        // Remove elements outside the window
        while (!dq.empty() && dq.front() < i - k) {
            dq.pop_front();
        }
        
        // Calculate dp[i]
        dp[i] = nums[i];
        if (!dq.empty()) {
            dp[i] = max(dp[i], nums[i] + dp[dq.front()]);
        }
        
        // Maintain decreasing deque
        while (!dq.empty() && dp[dq.back()] <= dp[i]) {
            dq.pop_back();
        }
        dq.push_back(i);
        
        maxSum = max(maxSum, dp[i]);
    }
    
    return maxSum;
}

int main() {
    // Test case 1
    vector<int> nums1 = {10, 2, -10, 5, 20};
    int k1 = 2;
    cout << "Test 1: " << constrainedSubsetSum(nums1, k1) << endl; // Expected: 37
    
    // Test case 2
    vector<int> nums2 = {-1, -2, -3};
    int k2 = 1;
    cout << "Test 2: " << constrainedSubsetSum(nums2, k2) << endl; // Expected: -1
    
    // Test case 3
    vector<int> nums3 = {10, -2, -10, -5, 20};
    int k3 = 2;
    cout << "Test 3: " << constrainedSubsetSum(nums3, k3) << endl; // Expected: 23
    
    // Test case 4
    vector<int> nums4 = {1, 2, 3, 4, 5};
    int k4 = 3;
    cout << "Test 4: " << constrainedSubsetSum(nums4, k4) << endl; // Expected: 15
    
    // Test case 5
    vector<int> nums5 = {5, -3, 5};
    int k5 = 1;
    cout << "Test 5: " << constrainedSubsetSum(nums5, k5) << endl; // Expected: 10
    
    return 0;
}
