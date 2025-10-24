#include <bits/stdc++.h>
using namespace std;

vector<int> maxSlidingWindow(const vector<int>& nums, int k) {
    int n = (int)nums.size();
    if (k == 0 || n == 0) return {};
    deque<int> dq;
    vector<int> res;
    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();
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

int main() {
    vector<int> nums1 = {1,3,-1,-3,5,3,6,7}; int k1 = 3;
    vector<int> nums2 = {1}; int k2 = 1;
    vector<int> nums3 = {9,8,7,6,5}; int k3 = 2;
    vector<int> nums4 = {-1,-3,-5,-2,-1}; int k4 = 3;
    vector<int> nums5 = {4,2,12,11,-5,6,2}; int k5 = 4;

    printVec(maxSlidingWindow(nums1, k1)); // [3,3,5,5,6,7]
    printVec(maxSlidingWindow(nums2, k2)); // [1]
    printVec(maxSlidingWindow(nums3, k3)); // [9,8,7,6]
    printVec(maxSlidingWindow(nums4, k4)); // [-1,-2,-1]
    printVec(maxSlidingWindow(nums5, k5)); // [12,12,12,11]
    return 0;
}