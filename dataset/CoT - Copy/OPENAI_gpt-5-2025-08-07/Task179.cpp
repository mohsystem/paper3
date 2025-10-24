#include <bits/stdc++.h>
using namespace std;

// Steps 1-4: Implement robust solution with checks
vector<int> maxSlidingWindow(const vector<int>& nums, int k) {
    if (nums.empty() || k <= 0 || k > (int)nums.size()) return {};
    int n = (int)nums.size();
    deque<int> dq; // indices
    vector<int> res;
    res.reserve(n - k + 1);

    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) res.push_back(nums[dq.front()]);
    }
    return res;
}

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ",";
        cout << v[i];
    }
    cout << "]\n";
}

// Step 5: tests
int main() {
    vector<pair<vector<int>, int>> tests = {
        {{1,3,-1,-3,5,3,6,7}, 3},
        {{1}, 1},
        {{9,10,9,-7,-4,-8,2,-6}, 5},
        {{1,-1}, 1},
        {{7,2,4}, 2}
    };
    for (auto& tc : tests) {
        auto res = maxSlidingWindow(tc.first, tc.second);
        printVec(res);
    }
    return 0;
}