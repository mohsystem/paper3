#include <bits/stdc++.h>
using namespace std;

static const int MIN_VAL = -10000;
static const int MAX_VAL = 10000;

vector<int> countSmaller(const vector<int>& nums) {
    if (nums.size() > 100000ULL) {
        throw invalid_argument("Invalid array length.");
    }
    for (int v : nums) {
        if (v < MIN_VAL || v > MAX_VAL) {
            throw invalid_argument("Value out of allowed range.");
        }
    }
    size_t n = nums.size();
    if (n == 0) return {};

    vector<int> vals(nums.begin(), nums.end());
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    int m = static_cast<int>(vals.size());

    vector<int> bit(m + 2, 0);
    auto add = [&](int idx, int delta) {
        while (idx < (int)bit.size()) {
            bit[idx] += delta;
            idx += idx & -idx;
        }
    };
    auto query = [&](int idx) {
        int s = 0;
        while (idx > 0) {
            s += bit[idx];
            idx -= idx & -idx;
        }
        return s;
    };

    vector<int> res(n, 0);
    for (int i = (int)n - 1; i >= 0; --i) {
        int idx = int(lower_bound(vals.begin(), vals.end(), nums[i]) - vals.begin()) + 1;
        res[i] = query(idx - 1);
        add(idx, 1);
    }
    return res;
}

static void printArray(const vector<int>& arr) {
    cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) cout << ",";
        cout << arr[i];
    }
    cout << "]\n";
}

int main() {
    try {
        vector<vector<int>> tests = {
            {5, 2, 6, 1},
            {-1},
            {-1, -1},
            {1, 2, 3},
            {3, 2, 1}
        };
        for (const auto& t : tests) {
            auto ans = countSmaller(t);
            printArray(ans);
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}