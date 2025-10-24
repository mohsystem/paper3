#include <bits/stdc++.h>
using namespace std;

long long constrainedSubsetSum(const vector<int>& nums, int k) {
    if (nums.empty() || k <= 0) {
        throw invalid_argument("Invalid input");
    }
    int n = (int)nums.size();
    vector<long long> dp(n, 0);
    deque<int> dq; // store indices with decreasing dp values
    long long ans = LLONG_MIN;

    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && dq.front() < i - k) dq.pop_front();
        long long bestPrev = (!dq.empty() && dp[dq.front()] > 0) ? dp[dq.front()] : 0;
        dp[i] = (long long)nums[i] + bestPrev;
        ans = max(ans, dp[i]);

        while (!dq.empty() && dp[dq.back()] <= dp[i]) dq.pop_back();
        dq.push_back(i);
    }
    return ans;
}

int main() {
    vector<pair<vector<int>, int>> tests = {
        {{10, 2, -10, 5, 20}, 2},
        {{-1, -2, -3}, 1},
        {{10, -2, -10, -5, 20}, 2},
        {{1, -1, -2, 4, -7, 3}, 2},
        {{-5}, 1}
    };
    vector<long long> expected = {37, -1, 23, 7, -5};

    for (size_t i = 0; i < tests.size(); ++i) {
        long long res = constrainedSubsetSum(tests[i].first, tests[i].second);
        cout << "Result " << (i + 1) << ": " << res << " (Expected: " << expected[i] << ")\n";
    }
    return 0;
}