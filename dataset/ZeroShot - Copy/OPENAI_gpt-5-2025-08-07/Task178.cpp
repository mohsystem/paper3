#include <bits/stdc++.h>
using namespace std;

class Task178 {
public:
    // Returns the length of the shortest non-empty subarray with sum at least k, or -1 if none exists.
    static int shortestSubarray(const vector<int>& nums, long long k) {
        if (nums.empty()) return -1;
        int n = (int)nums.size();
        vector<long long> prefix(n + 1, 0);
        for (int i = 0; i < n; ++i) {
            prefix[i + 1] = prefix[i] + (long long)nums[i];
        }
        deque<int> dq;
        int ans = n + 1;
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
        return (ans <= n) ? ans : -1;
    }
};

int main() {
    cout << Task178::shortestSubarray(vector<int>{1}, 1) << "\n"; // 1
    cout << Task178::shortestSubarray(vector<int>{1, 2}, 4) << "\n"; // -1
    cout << Task178::shortestSubarray(vector<int>{2, -1, 2}, 3) << "\n"; // 3
    cout << Task178::shortestSubarray(vector<int>{84, -37, 32, 40, 95}, 167) << "\n"; // 3
    cout << Task178::shortestSubarray(vector<int>{17, 85, 93, -45, -21}, 150) << "\n"; // 2
    return 0;
}