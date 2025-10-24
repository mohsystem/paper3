#include <bits/stdc++.h>
using namespace std;

int findOutlier(const vector<int>& arr) {
    int evenCount = 0;
    int limit = min(3, (int)arr.size());
    for (int i = 0; i < limit; ++i) {
        if ((arr[i] & 1) == 0) evenCount++;
    }
    bool majorityEven = evenCount >= 2;
    for (int x : arr) {
        if (majorityEven && (x & 1)) return x;
        if (!majorityEven && ((x & 1) == 0)) return x;
    }
    return 0; // Should not happen for valid inputs
}

int main() {
    vector<vector<int>> tests = {
        {2, 4, 0, 100, 4, 11, 2602, 36},
        {160, 3, 1719, 19, 11, 13, -21},
        {1, 1, 2},
        {2, 6, 8, 10, 3},
        {-2, -4, -6, -7, -8}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << " outlier: " << findOutlier(tests[i]) << "\n";
    }
    return 0;
}