#include <bits/stdc++.h>
using namespace std;

vector<int> two_sum(const vector<int>& nums, int target) {
    unordered_map<int, int> mp;
    for (int i = 0; i < (int)nums.size(); ++i) {
        int comp = target - nums[i];
        auto it = mp.find(comp);
        if (it != mp.end()) {
            return {it->second, i};
        }
        if (mp.find(nums[i]) == mp.end()) {
            mp[nums[i]] = i;
        }
    }
    return {};
}

int main() {
    vector<pair<vector<int>, int>> tests = {
        {{1, 2, 3}, 4},
        {{3, 2, 4}, 6},
        {{2, 7, 11, 15}, 9},
        {{3, 3}, 6},
        {{0, -1, 2, -3, 5}, -1}
    };

    for (auto& test : tests) {
        vector<int> res = two_sum(test.first, test.second);
        if (res.size() == 2) {
            cout << "{" << res[0] << ", " << res[1] << "}" << "\n";
        } else {
            cout << "{}\n";
        }
    }
    return 0;
}