#include <bits/stdc++.h>
using namespace std;

// Step 1: Problem understanding
// Find the lowest index N where sum of left elements equals sum of right elements.

// Step 2-3: Security requirements and Secure coding generation
// - Use long long for sums to reduce overflow risk.
// - No unsafe memory operations.

int findEvenIndex(const vector<int>& arr) {
    long long total = 0;
    for (int v : arr) total += v;
    long long left = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        long long right = total - left - arr[i];
        if (left == right) return static_cast<int>(i);
        left += arr[i];
    }
    return -1;
}

// Step 4: Code review
// - Indexing safe with size_t and bounds.
// - Casting to int safe since index < arr.size() <= reasonable constraints.

// Step 5: Secure code output
int main() {
    vector<vector<int>> tests = {
        {1,2,3,4,3,2,1},              // expected 3
        {1,100,50,-51,1,1},           // expected 1
        {20,10,-80,10,10,15,35},      // expected 0
        {1,2,3,4,5,6},                // expected -1
        {2,0,2}                       // expected 1
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        int res = findEvenIndex(tests[i]);
        cout << "Test " << (i + 1) << " [";
        for (size_t j = 0; j < tests[i].size(); ++j) {
            cout << tests[i][j] << (j + 1 < tests[i].size() ? "," : "");
        }
        cout << "] => " << res << "\n";
    }
    return 0;
}