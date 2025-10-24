#include <iostream>
#include <vector>
using namespace std;

int findOdd(const vector<int>& arr) {
    int x = 0;
    for (int v : arr) x ^= v;
    return x;
}

int main() {
    vector<vector<int>> tests = {
        {7},
        {0},
        {1, 1, 2},
        {0, 1, 0, 1, 0},
        {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}
    };
    for (const auto& t : tests) {
        cout << findOdd(t) << endl;
    }
    return 0;
}