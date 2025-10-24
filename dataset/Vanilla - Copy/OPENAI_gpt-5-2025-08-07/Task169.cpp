#include <bits/stdc++.h>
using namespace std;

class Task169 {
public:
    static vector<int> countSmaller(const vector<int>& nums) {
        int n = (int)nums.size();
        vector<int> res(n, 0);
        if (n == 0) return res;

        vector<int> vals = nums;
        sort(vals.begin(), vals.end());
        vals.erase(unique(vals.begin(), vals.end()), vals.end());
        int m = (int)vals.size();

        vector<int> bit(m + 2, 0);

        auto idx = [&](int v) {
            int id = int(lower_bound(vals.begin(), vals.end(), v) - vals.begin()) + 1;
            return id;
        };

        auto update = [&](int i, int delta) {
            for (; i <= m + 1; i += i & -i) bit[i] += delta;
        };

        auto query = [&](int i) {
            int s = 0;
            for (; i > 0; i -= i & -i) s += bit[i];
            return s;
        };

        for (int i = n - 1; i >= 0; --i) {
            int id = idx(nums[i]);
            res[i] = query(id - 1);
            update(id, 1);
        }
        return res;
    }
};

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ",";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    vector<vector<int>> tests = {
        {5,2,6,1},
        {-1},
        {-1,-1},
        {3,2,2,6,1},
        {1,0,2,2,5}
    };
    for (auto& t : tests) {
        auto res = Task169::countSmaller(t);
        printVec(res);
    }
    return 0;
}