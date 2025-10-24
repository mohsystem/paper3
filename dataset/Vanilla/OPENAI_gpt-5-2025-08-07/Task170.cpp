#include <bits/stdc++.h>
using namespace std;

long long sortCount(vector<long long>& sums, vector<long long>& tmp, int lo, int hi, long long lower, long long upper) {
    if (hi - lo <= 1) return 0;
    int mid = (lo + hi) >> 1;
    long long cnt = sortCount(sums, tmp, lo, mid, lower, upper) + sortCount(sums, tmp, mid, hi, lower, upper);
    int l = mid, r = mid;
    for (int i = lo; i < mid; ++i) {
        while (l < hi && sums[l] - sums[i] < lower) ++l;
        while (r < hi && sums[r] - sums[i] <= upper) ++r;
        cnt += r - l;
    }
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = (sums[i] <= sums[j]) ? sums[i++] : sums[j++];
    while (i < mid) tmp[k++] = sums[i++];
    while (j < hi) tmp[k++] = sums[j++];
    for (i = lo; i < hi; ++i) sums[i] = tmp[i];
    return cnt;
}

long long countRangeSum(const vector<int>& nums, long long lower, long long upper) {
    int n = nums.size();
    vector<long long> sums(n + 1, 0), tmp(n + 1, 0);
    for (int i = 0; i < n; ++i) sums[i + 1] = sums[i] + nums[i];
    return sortCount(sums, tmp, 0, n + 1, lower, upper);
}

int main() {
    vector<vector<int>> testsNums = {
        {-2, 5, -1},
        {0},
        {1, -1, 2, -2, 3},
        {0, 0, 0},
        {2, -2, 2, -2}
    };
    vector<long long> lowers = {-2, 0, 1, 0, -1};
    vector<long long> uppers = {2, 0, 3, 0, 1};
    for (int t = 0; t < (int)testsNums.size(); ++t) {
        cout << countRangeSum(testsNums[t], lowers[t], uppers[t]) << "\n";
    }
    return 0;
}