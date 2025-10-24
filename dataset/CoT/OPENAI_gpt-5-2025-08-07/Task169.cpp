#include <bits/stdc++.h>
using namespace std;

vector<int> countSmaller(const vector<int>& nums) {
    int n = (int)nums.size();
    vector<int> res(n, 0);
    if (n == 0) return res;

    // Coordinate compression
    vector<int> uniq = nums;
    sort(uniq.begin(), uniq.end());
    uniq.erase(unique(uniq.begin(), uniq.end()), uniq.end());
    unordered_map<int,int> mp;
    mp.reserve(uniq.size() * 2);
    for (int i = 0; i < (int)uniq.size(); ++i) {
        mp[uniq[i]] = i + 1; // 1-based index for BIT
    }

    // Fenwick Tree
    vector<int> bit(uniq.size() + 3, 0);
    auto update = [&](int idx, int delta) {
        for (int i = idx; i < (int)bit.size(); i += i & -i) {
            bit[i] += delta;
        }
    };
    auto query = [&](int idx) {
        int s = 0;
        for (int i = idx; i > 0; i -= i & -i) {
            s += bit[i];
        }
        return s;
    };

    for (int i = n - 1; i >= 0; --i) {
        int idx = mp[nums[i]];
        res[i] = query(idx - 1);
        update(idx, 1);
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
    // 5 test cases
    vector<vector<int>> tests = {
        {5,2,6,1},
        {-1},
        {-1,-1},
        {1,2,3,4},
        {4,3,2,1}
    };
    for (auto &t : tests) {
        vector<int> res = countSmaller(t);
        printVec(res);
    }
    return 0;
}