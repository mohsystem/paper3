// Chain-of-Through:
// 1) Understand: count pairs (i<j) with nums[i] > 2*nums[j]
// 2) Security: use long long to avoid overflow
// 3) Secure coding: bounds-safe indices, no UB
// 4) Review: comparisons with 2LL*
// 5) Output: 5 tests

#include <iostream>
#include <vector>
#include <cstdlib>

static long long countPairs(std::vector<long long>& a, int l, int m, int r) {
    long long cnt = 0;
    int j = m + 1;
    for (int i = l; i <= m; ++i) {
        while (j <= r && a[i] > 2LL * a[j]) {
            ++j;
        }
        cnt += (j - (m + 1));
    }
    return cnt;
}

static void merge(std::vector<long long>& a, int l, int m, int r, std::vector<long long>& temp) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i <= m) temp[k++] = a[i++];
    while (j <= r) temp[k++] = a[j++];
    for (int t = l; t <= r; ++t) a[t] = temp[t];
}

static long long sortAndCount(std::vector<long long>& a, int l, int r, std::vector<long long>& temp) {
    if (l >= r) return 0;
    int m = l + (r - l) / 2;
    long long cnt = 0;
    cnt += sortAndCount(a, l, m, temp);
    cnt += sortAndCount(a, m + 1, r, temp);
    cnt += countPairs(a, l, m, r);
    merge(a, l, m, r, temp);
    return cnt;
}

long long reversePairs(const std::vector<long long>& nums) {
    if (nums.size() < 2) return 0LL;
    std::vector<long long> a(nums.begin(), nums.end());
    std::vector<long long> temp(a.size(), 0);
    return sortAndCount(a, 0, static_cast<int>(a.size()) - 1, temp);
}

int main() {
    std::vector<std::vector<long long>> tests = {
        {1, 3, 2, 3, 1},
        {2, 4, 3, 5, 1},
        {5, 4, 3, 2, 1},
        {1, 1, 1, 1},
        {-5, -5}
    };
    for (const auto& t : tests) {
        std::cout << reversePairs(t) << std::endl;
    }
    return 0;
}