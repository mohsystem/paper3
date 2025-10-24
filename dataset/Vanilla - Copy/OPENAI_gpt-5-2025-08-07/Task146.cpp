#include <bits/stdc++.h>
using namespace std;

int findMissingNumber(const vector<int>& arr) {
    int n = static_cast<int>(arr.size()) + 1;
    int xr = 0;
    for (int i = 1; i <= n; ++i) xr ^= i;
    for (int v : arr) xr ^= v;
    return xr;
}

int main() {
    vector<vector<int>> tests = {
        {2, 3, 1, 5},
        {1},
        {2},
        {1, 2, 3, 4, 5, 6, 7, 8, 10},
        {}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << " missing number: " << findMissingNumber(tests[i]) << "\n";
    }
    return 0;
}