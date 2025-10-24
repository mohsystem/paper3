// Chain-of-Through Steps:
// 1) Problem understanding: Count number of subarray sums in [lower, upper] using prefix sums and modified merge sort.
// 2) Security requirements: Use 64-bit integers for sums; avoid overflow; no uninitialized access.
// 3) Secure coding: Pass by const reference; pre-allocate temp buffer; bounds-checked loops.
// 4) Code review: Correct merge and count windows, copy back safely.
// 5) Secure code output: Clean, efficient, and safe.

#include <bits/stdc++.h>
using namespace std;

int countRangeSum(const vector<int>& nums, long long lower, long long upper) {
    int n = (int)nums.size();
    if (n == 0) return 0;
    vector<long long> sums(n + 1, 0);
    for (int i = 0; i < n; ++i) sums[i + 1] = sums[i] + (long long)nums[i];
    vector<long long> temp(n + 1, 0);

    function<long long(int,int)> sort_count = [&](int lo, int hi) -> long long {
        if (hi - lo <= 1) return 0LL;
        int mid = (lo + hi) / 2;
        long long cnt = sort_count(lo, mid) + sort_count(mid, hi);
        int j = mid, k = mid, t = mid;
        int r = lo;
        for (int i = lo; i < mid; ++i) {
            while (k < hi && sums[k] - sums[i] < lower) ++k;
            while (j < hi && sums[j] - sums[i] <= upper) ++j;
            while (t < hi && sums[t] < sums[i]) temp[r++] = sums[t++];
            temp[r++] = sums[i];
            cnt += (j - k);
        }
        while (t < hi) temp[r++] = sums[t++];
        for (int p = lo; p < hi; ++p) sums[p] = temp[p];
        return cnt;
    };

    long long res = sort_count(0, n + 1);
    return (int)res; // guaranteed to fit in 32-bit
}

int main() {
    vector<pair<vector<int>, pair<long long,long long>>> tests = {
        {{-2, 5, -1}, {-2, 2}},                     // expected 3
        {{0}, {0, 0}},                              // expected 1
        {{1, -1, 1}, {0, 1}},                       // expected 5
        {{2147483647, -2147483648, -1, 0}, {-1, 0}},// expected 4
        {{1, 2, 3}, {3, 6}}                         // expected 4
    };
    for (auto& t : tests) {
        cout << countRangeSum(t.first, t.second.first, t.second.second) << "\n";
    }
    return 0;
}