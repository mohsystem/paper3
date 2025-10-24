#include <bits/stdc++.h>
using namespace std;

vector<int> removeSmallest(const vector<int>& arr) {
    if (arr.empty()) return {};
    int minVal = arr[0];
    size_t minIdx = 0;
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIdx = i;
        }
    }
    vector<int> res;
    res.reserve(arr.size() - 1);
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i != minIdx) res.push_back(arr[i]);
    }
    return res;
}

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        cout << v[i];
        if (i + 1 < v.size()) cout << ",";
    }
    cout << "]\n";
}

int main() {
    vector<vector<int>> tests = {
        {1,2,3,4,5},
        {5,3,2,1,4},
        {2,2,1,2,1},
        {},
        {7,7,7}
    };
    for (const auto& t : tests) {
        auto out = removeSmallest(t);
        printVec(out);
    }
    return 0;
}