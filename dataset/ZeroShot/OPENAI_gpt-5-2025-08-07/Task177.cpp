#include <bits/stdc++.h>
using namespace std;

long long constrainedSubsetSum(const vector<int>& nums, int k) {
    if (nums.empty() || k <= 0) return 0LL;
    int n = (int)nums.size();
    vector<long long> dp(n);
    deque<int> dq; // indices of dp, decreasing dp values
    long long ans = LLONG_MIN;
    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && dq.front() < i - k) dq.pop_front();
        long long best = dq.empty() ? 0LL : dp[dq.front()];
        dp[i] = (long long)nums[i] + max(0LL, best);
        ans = max(ans, dp[i]);
        while (!dq.empty() && dp[dq.back()] <= dp[i]) dq.pop_back();
        dq.push_back(i);
    }
    return ans;
}

static void runTest(const vector<int>& nums, int k) {
    cout << constrainedSubsetSum(nums, k) << "\n";
}

int main() {
    runTest({10, 2, -10, 5, 20}, 2);
    runTest({-1, -2, -3}, 1);
    runTest({10, -2, -10, -5, 20}, 2);
    runTest({1, -1, -2, 4, -7, 3}, 2);
    runTest({5, -1, 5}, 1);
    return 0;
}