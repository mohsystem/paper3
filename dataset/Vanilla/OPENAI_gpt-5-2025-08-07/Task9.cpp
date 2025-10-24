#include <bits/stdc++.h>
using namespace std;

string odd_or_even(const vector<long long>& arr) {
    long long parity = 0;
    if (arr.empty()) return "even";
    for (auto x : arr) {
        parity ^= (llabs(x) & 1LL);
    }
    return parity == 0 ? "even" : "odd";
}

int main() {
    vector<vector<long long>> tests = {
        {},
        {0},
        {0, 1, 4},
        {0, -1, -5},
        {1, 2, 3, 4, 5}
    };
    for (const auto& t : tests) {
        cout << odd_or_even(t) << "\n";
    }
    return 0;
}