#include <bits/stdc++.h>
using namespace std;

long long mergeSortCount(vector<long long>& arr, vector<long long>& temp, int left, int right) {
    if (left >= right) return 0LL;
    int mid = left + (right - left) / 2;
    long long count = 0LL;
    count += mergeSortCount(arr, temp, left, mid);
    count += mergeSortCount(arr, temp, mid + 1, right);

    int j = mid + 1;
    for (int i = left; i <= mid; ++i) {
        while (j <= right && arr[i] > 2LL * arr[j]) ++j;
        count += (j - (mid + 1));
    }

    int i = left; j = mid + 1; int k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (int idx = left; idx <= right; ++idx) arr[idx] = temp[idx];

    return count;
}

long long countReversePairs(const vector<long long>& nums) {
    if (nums.size() <= 1) return 0LL;
    vector<long long> arr(nums.begin(), nums.end());
    vector<long long> temp(arr.size());
    return mergeSortCount(arr, temp, 0, (int)arr.size() - 1);
}

int main() {
    vector<vector<long long>> tests = {
        {1, 3, 2, 3, 1},
        {2, 4, 3, 5, 1},
        {1},
        {2147483647LL, -2147483648LL, 0LL},
        {-5LL, -5LL}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << " result: " << countReversePairs(tests[i]) << "\n";
    }
    return 0;
}