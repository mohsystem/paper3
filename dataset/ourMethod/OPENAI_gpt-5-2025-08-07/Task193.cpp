#include <bits/stdc++.h>
using namespace std;

bool aliceWins(const vector<int>& nums) {
    // Validate inputs per constraints
    if (nums.empty() || nums.size() > 1000) {
        return false; // fail closed on invalid input
    }
    int xr = 0;
    for (int v : nums) {
        if (v < 0 || v >= (1 << 16)) {
            return false; // fail closed on invalid value
        }
        xr ^= v;
    }
    // If xor is 0 at start, Alice wins. Else Alice wins iff length is even.
    return xr == 0 || (nums.size() % 2 == 0);
}

void runTest(const vector<int>& nums) {
    cout << "nums=[";
    for (size_t i = 0; i < nums.size(); ++i) {
        if (i) cout << ",";
        cout << nums[i];
    }
    cout << "] -> " << boolalpha << aliceWins(nums) << "\n";
}

int main() {
    // 5 test cases
    runTest({1, 1, 2});    // false
    runTest({0, 1});       // true
    runTest({1, 2, 3});    // true
    runTest({5});          // false
    runTest({1, 1, 1});    // false
    return 0;
}