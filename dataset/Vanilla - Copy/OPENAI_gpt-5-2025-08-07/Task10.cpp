#include <bits/stdc++.h>
using namespace std;

long long getSum(long long a, long long b) {
    long long lo = min(a, b);
    long long hi = max(a, b);
    long long n = hi - lo + 1;
    if ((n & 1LL) == 0LL) {
        return (n / 2) * (lo + hi);
    } else {
        return n * ((lo + hi) / 2);
    }
}

int main() {
    vector<pair<long long, long long>> tests = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };
    for (auto &p : tests) {
        cout << getSum(p.first, p.second) << "\n";
    }
    return 0;
}