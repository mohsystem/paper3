#include <bits/stdc++.h>
using namespace std;

bool xorGame(const vector<int>& nums) {
    int xr = 0;
    for (int v : nums) xr ^= v;
    if (xr == 0) return true;
    return nums.size() % 2 == 0;
}

int main() {
    vector<vector<int>> tests = {
        {1, 1, 2},
        {0, 1},
        {1, 2, 3},
        {2},
        {1, 1}
    };
    for (auto& t : tests) {
        cout << (xorGame(t) ? "true" : "false") << "\n";
    }
    return 0;
}