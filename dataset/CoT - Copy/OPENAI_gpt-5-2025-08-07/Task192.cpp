// Chain-of-Through Process:
// 1. Problem understanding: Determine if Alice wins given XOR game rules.
// 2. Security requirements: Avoid undefined behavior; handle empty input defensively.
// 3. Secure coding generation: No dynamic allocation beyond std::vector; pure compute.
// 4. Code review: XOR reduce and parity logic.
// 5. Secure code output: Final function and tests.

#include <bits/stdc++.h>
using namespace std;

bool xorGame(const vector<int>& nums) {
    if (nums.empty()) {
        // XOR of no elements is 0; starting player wins.
        return true;
    }
    int x = 0;
    for (int v : nums) {
        x ^= (v & 0xFFFF);
    }
    if (x == 0) return true;
    return (nums.size() % 2 == 0);
}

static void runTest(const vector<int>& nums) {
    cout << "nums=[";
    for (size_t i = 0; i < nums.size(); ++i) {
        if (i) cout << ", ";
        cout << nums[i];
    }
    cout << "] -> " << (xorGame(nums) ? "true" : "false") << "\n";
}

int main() {
    runTest({1, 1, 2});   // false
    runTest({0, 1});      // true
    runTest({1, 2, 3});   // true
    runTest({1});         // false
    runTest({1, 1});      // true
    return 0;
}