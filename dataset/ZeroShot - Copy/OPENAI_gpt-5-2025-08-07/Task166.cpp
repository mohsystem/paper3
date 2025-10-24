#include <bits/stdc++.h>
using namespace std;

vector<int> posNegSort(const vector<int>& arr) {
    vector<int> res = arr;
    vector<int> pos;
    pos.reserve(arr.size());
    for (int v : arr) if (v > 0) pos.push_back(v);
    sort(pos.begin(), pos.end());
    size_t pi = 0;
    for (size_t i = 0; i < res.size(); ++i) {
        if (res[i] > 0) {
            if (pi < pos.size()) res[i] = pos[pi++];
        }
    }
    return res;
}

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    vector<vector<int>> tests = {
        {6, 3, -2, 5, -8, 2, -2},
        {6, 5, 4, -1, 3, 2, -1, 1},
        {-5, -5, -5, -5, 7, -5},
        {},
        {5, -1, -3, 2, 1}
    };
    for (const auto& t : tests) {
        auto out = posNegSort(t);
        printVec(out);
    }
    return 0;
}