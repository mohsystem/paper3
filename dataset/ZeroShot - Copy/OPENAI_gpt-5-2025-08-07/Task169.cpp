#include <bits/stdc++.h>
using namespace std;

static vector<int> countSmaller(const vector<int>& nums) {
    size_t n = nums.size();
    if (n == 0) return {};
    vector<int> sorted = nums;
    sort(sorted.begin(), sorted.end());
    sorted.erase(unique(sorted.begin(), sorted.end()), sorted.end());

    auto getRank = [&](int x) {
        return int(lower_bound(sorted.begin(), sorted.end(), x) - sorted.begin()) + 1; // 1-based
    };

    int m = (int)sorted.size();
    vector<int> bit(m + 1, 0);
    auto update = [&](int idx, int delta) {
        for (int i = idx; i <= m; i += i & -i) bit[i] += delta;
    };
    auto query = [&](int idx) {
        int s = 0;
        for (int i = idx; i > 0; i -= i & -i) s += bit[i];
        return s;
    };

    vector<int> ans(n, 0);
    for (int i = (int)n - 1; i >= 0; --i) {
        int r = getRank(nums[i]);
        ans[i] = query(r - 1);
        update(r, 1);
    }
    return ans;
}

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ",";
        cout << v[i];
    }
    cout << "]";
}

int main() {
    vector<vector<int>> tests = {
        {5,2,6,1},
        {-1},
        {-1,-1},
        {3,2,2,6,1},
        {2,0,1}
    };
    for (const auto& t : tests) {
        vector<int> res = countSmaller(t);
        cout << "Input: "; printVec(t); cout << "\n";
        cout << "Output: "; printVec(res); cout << "\n";
        cout << "---\n";
    }
    return 0;
}