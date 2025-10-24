#include <bits/stdc++.h>
using namespace std;

class Task166 {
public:
    static vector<int> posNegSort(const vector<int>& arr) {
        vector<int> res = arr;
        vector<int> pos;
        for (int x : arr) if (x > 0) pos.push_back(x);
        sort(pos.begin(), pos.end());
        size_t idx = 0;
        for (size_t i = 0; i < arr.size(); ++i) {
            if (arr[i] > 0) res[i] = pos[idx++];
            else res[i] = arr[i];
        }
        return res;
    }
};

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
        {9, -3, 8, -1, -2, 7, 6}
    };
    for (const auto& t : tests) {
        printVec(Task166::posNegSort(t));
    }
    return 0;
}