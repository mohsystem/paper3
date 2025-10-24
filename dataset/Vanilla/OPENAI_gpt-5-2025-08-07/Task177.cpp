#include <bits/stdc++.h>
using namespace std;

int constrainedSubsetSum(const vector<int>& nums, int k) {
    int n = (int)nums.size();
    vector<int> dp(n, 0);
    deque<int> dq; // indices with dp decreasing
    int ans = INT_MIN;

    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && i - dq.front() > k) dq.pop_front();
        int bestPrev = dq.empty() ? 0 : max(0, dp[dq.front()]);
        dp[i] = nums[i] + bestPrev;
        ans = max(ans, dp[i]);
        if (dp[i] > 0) {
            while (!dq.empty() && dp[dq.back()] <= dp[i]) dq.pop_back();
            dq.push_back(i);
        }
    }
    return ans;
}

int main() {
    vector<pair<vector<int>, int>> tests = {
        {{10, 2, -10, 5, 20}, 2},
        {{-1, -2, -3}, 1},
        {{10, -2, -10, -5, 20}, 2},
        {{1, -1, -1, -1, 5}, 2},
        {{5, -1, -2, 10}, 3},
    };
    for (auto& p : tests) {
        cout << constrainedSubsetSum(p.first, p.second) << "\n";
    }
    return 0;
}