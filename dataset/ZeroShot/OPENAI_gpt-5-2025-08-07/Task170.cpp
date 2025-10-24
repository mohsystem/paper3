#include <bits/stdc++.h>
using namespace std;

static long long sortAndCount(vector<long long>& sums, vector<long long>& temp, int left, int right, long long lower, long long upper) {
    if (right - left <= 1) return 0LL;
    int mid = left + ((right - left) >> 1);
    long long count = 0;
    count += sortAndCount(sums, temp, left, mid, lower, upper);
    count += sortAndCount(sums, temp, mid, right, lower, upper);

    int j = mid, k = mid, r = mid, p = left;
    for (int i = left; i < mid; ++i) {
        while (j < right && sums[j] - sums[i] < lower) ++j;
        while (k < right && sums[k] - sums[i] <= upper) ++k;
        count += (k - j);
        while (r < right && sums[r] < sums[i]) temp[p++] = sums[r++];
        temp[p++] = sums[i];
    }
    while (r < right) temp[p++] = sums[r++];
    for (int i = left; i < right; ++i) sums[i] = temp[i];
    return count;
}

int countRangeSum(const vector<int>& nums, long long lower, long long upper) {
    int n = (int)nums.size();
    vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + (long long)nums[i];
    vector<long long> temp(n + 1, 0);
    long long cnt = sortAndCount(prefix, temp, 0, n + 1, lower, upper);
    if (cnt > INT_MAX) return INT_MAX;
    if (cnt < INT_MIN) return INT_MIN;
    return (int)cnt;
}

int main() {
    vector<vector<int>> tests = {
        {-2, 5, -1},
        {0},
        {1, 2, 3, 4},
        {-1, -1, -1},
        {0, 0, 0, 0}
    };
    vector<long long> lowers = {-2, 0, 3, -2, 0};
    vector<long long> uppers = {2, 0, 6, -1, 0};
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << countRangeSum(tests[i], lowers[i], uppers[i]) << "\n";
    }
    return 0;
}