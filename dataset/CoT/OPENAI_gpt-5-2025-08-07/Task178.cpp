#include <iostream>
#include <vector>
#include <deque>
using namespace std;

int shortestSubarray(const vector<int>& nums, long long k) {
    int n = (int)nums.size();
    vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + nums[i];
    deque<int> dq;
    int ans = n + 1;
    for (int i = 0; i <= n; ++i) {
        while (!dq.empty() && prefix[i] - prefix[dq.front()] >= k) {
            ans = min(ans, i - dq.front());
            dq.pop_front();
        }
        while (!dq.empty() && prefix[i] <= prefix[dq.back()]) dq.pop_back();
        dq.push_back(i);
    }
    return (ans <= n) ? ans : -1;
}

int main() {
    vector<pair<vector<int>, long long>> tests = {
        {{1}, 1},
        {{1, 2}, 4},
        {{2, -1, 2}, 3},
        {{84, -37, 32, 40, 95}, 167},
        {{17, 85, 93, -45, -21}, 150}
    };
    for (auto& t : tests) {
        cout << shortestSubarray(t.first, t.second) << "\n";
    }
    return 0;
}