#include <bits/stdc++.h>
using namespace std;

int longestRun(const vector<int>& arr) {
    if (arr.empty()) return 0;
    int maxLen = 1, curLen = 1, dir = 0; // dir: 1 inc, -1 dec, 0 unknown
    for (size_t i = 1; i < arr.size(); ++i) {
        int diff = arr[i] - arr[i - 1];
        if (diff == 1) {
            if (dir == 1 || dir == 0) curLen++;
            else curLen = 2;
            dir = 1;
        } else if (diff == -1) {
            if (dir == -1 || dir == 0) curLen++;
            else curLen = 2;
            dir = -1;
        } else {
            curLen = 1;
            dir = 0;
        }
        if (curLen > maxLen) maxLen = curLen;
    }
    return maxLen;
}

int main() {
    cout << longestRun({1, 2, 3, 5, 6, 7, 8, 9}) << "\n"; // 5
    cout << longestRun({1, 2, 3, 10, 11, 15}) << "\n";     // 3
    cout << longestRun({5, 4, 2, 1}) << "\n";              // 2
    cout << longestRun({3, 5, 7, 10, 15}) << "\n";         // 1
    cout << longestRun({7}) << "\n";                       // 1
    return 0;
}