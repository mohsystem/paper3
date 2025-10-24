#include <bits/stdc++.h>
using namespace std;

long long sortCount(vector<long long>& a, int l, int r, vector<long long>& temp) {
    if (l >= r) return 0LL;
    int m = (l + r) >> 1;
    long long cnt = sortCount(a, l, m, temp) + sortCount(a, m + 1, r, temp);

    int j = m + 1;
    for (int i = l; i <= m; ++i) {
        while (j <= r && a[i] > 2LL * a[j]) j++;
        cnt += (j - (m + 1));
    }

    int i = l; j = m + 1; int k = l;
    while (i <= m && j <= r) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i <= m) temp[k++] = a[i++];
    while (j <= r) temp[k++] = a[j++];
    for (int t = l; t <= r; ++t) a[t] = temp[t];
    return cnt;
}

long long reversePairs(vector<long long> nums) {
    if (nums.empty()) return 0LL;
    vector<long long> temp(nums.size());
    return sortCount(nums, 0, (int)nums.size() - 1, temp);
}

int main() {
    cout << reversePairs({1, 3, 2, 3, 1}) << "\n";                 // 2
    cout << reversePairs({2, 4, 3, 5, 1}) << "\n";                 // 3
    cout << reversePairs({-5, -5}) << "\n";                        // 1
    cout << reversePairs({2147483647LL, -2147483648LL, 0LL}) << "\n"; // 2
    cout << reversePairs({5, 4, 3, 2, 1}) << "\n";                 // 4
    return 0;
}