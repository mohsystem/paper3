#include <iostream>
#include <vector>
#include <limits>
using namespace std;

vector<int> removeSmallest(const vector<int>& arr) {
    if (arr.empty()) return {};
    int minVal = arr[0];
    int minIdx = 0;
    for (int i = 1; i < (int)arr.size(); ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIdx = i;
        }
    }
    vector<int> res;
    res.reserve(arr.size() - 1);
    for (int i = 0; i < (int)arr.size(); ++i) {
        if (i == minIdx) continue;
        res.push_back(arr[i]);
    }
    return res;
}

void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ",";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    vector<vector<int>> tests = {
        {1,2,3,4,5},
        {5,3,2,1,4},
        {2,2,1,2,1},
        {},
        {10}
    };
    for (const auto& t : tests) {
        printVec(removeSmallest(t));
    }
    return 0;
}