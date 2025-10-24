#include <bits/stdc++.h>
using namespace std;

class Task163 {
public:
    static int longestRun(const vector<int>& arr) {
        if (arr.empty()) return 0;
        int maxLen = 1, currLen = 1;
        int prevDiffSet = 0; // 0: none, 1: set
        int prevDiff = 0;    // +1 or -1

        for (size_t i = 1; i < arr.size(); ++i) {
            int diff = arr[i] - arr[i - 1];
            if (diff == 1 || diff == -1) {
                if (prevDiffSet && diff == prevDiff) {
                    currLen++;
                } else {
                    currLen = 2;
                    prevDiff = diff;
                    prevDiffSet = 1;
                }
            } else {
                currLen = 1;
                prevDiffSet = 0;
            }
            if (currLen > maxLen) maxLen = currLen;
        }
        return maxLen;
    }
};

int main() {
    cout << Task163::longestRun({1, 2, 3, 5, 6, 7, 8, 9}) << "\n";  // 5
    cout << Task163::longestRun({1, 2, 3, 10, 11, 15}) << "\n";     // 3
    cout << Task163::longestRun({5, 4, 2, 1}) << "\n";              // 2
    cout << Task163::longestRun({3, 5, 7, 10, 15}) << "\n";         // 1
    cout << Task163::longestRun({1, 2, 1, 0, -1}) << "\n";          // 4
    return 0;
}