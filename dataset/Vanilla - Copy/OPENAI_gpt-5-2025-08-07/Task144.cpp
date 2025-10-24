#include <bits/stdc++.h>
using namespace std;

vector<int> mergeSortedArrays(const vector<int>& a, const vector<int>& b) {
    vector<int> res;
    res.reserve(a.size() + b.size());
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] <= b[j]) res.push_back(a[i++]);
        else res.push_back(b[j++]);
    }
    while (i < a.size()) res.push_back(a[i++]);
    while (j < b.size()) res.push_back(b[j++]);
    return res;
}

int main() {
    vector<vector<int>> A = {
        {1, 3, 5},
        {},
        {1, 1, 2, 2},
        {-5, -3, 0, 2},
        {1, 2, 7, 8, 9}
    };
    vector<vector<int>> B = {
        {2, 4, 6},
        {1, 2, 3},
        {1, 2, 2, 3},
        {-4, -1, 3},
        {3, 4}
    };

    for (int t = 0; t < 5; ++t) {
        vector<int> merged = mergeSortedArrays(A[t], B[t]);
        cout << "[";
        for (size_t i = 0; i < merged.size(); ++i) {
            if (i) cout << ", ";
            cout << merged[i];
        }
        cout << "]\n";
    }
    return 0;
}