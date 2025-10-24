#include <bits/stdc++.h>
using namespace std;

int shortestSubarray(const vector<int>& nums, long long k) {
    int n = (int)nums.size();
    if (n == 0 || n > 1'000'000) return -1;

    vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + nums.at(i);
    }

    int ans = n + 1;
    deque<int> dq;
    for (int i = 0; i <= n; ++i) {
        while (!dq.empty() && prefix[i] - prefix[dq.front()] >= k) {
            ans = min(ans, i - dq.front());
            dq.pop_front();
        }
        while (!dq.empty() && prefix[i] <= prefix[dq.back()]) {
            dq.pop_back();
        }
        dq.push_back(i);
    }

    return ans <= n ? ans : -1;
}

int main() {
    vector<pair<vector<int>, long long>> tests = {
        {{1}, 1},
        {{1, 2}, 4},
        {{2, -1, 2}, 3},
        {{84, -37, 32, 40, 95}, 167},
        {{17, 85, 93, -45, -21}, 150}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        const auto& arr = tests[i].first;
        long long k = tests[i].second;
        cout << "Test " << (i + 1) << " nums=[";
        for (size_t j = 0; j < arr.size(); ++j) {
            if (j) cout << ",";
            cout << arr[j];
        }
        cout << "] k=" << k << " -> " << shortestSubarray(arr, k) << "\n";
    }
    return 0;
}