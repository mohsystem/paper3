#include <bits/stdc++.h>
using namespace std;

long long randomGreaterThan(long long minExclusive) {
    static random_device rd;
    static mt19937_64 gen(rd());
    long long lower = minExclusive + 1;
    long long upper = (minExclusive <= LLONG_MAX - 1000001LL) ? (minExclusive + 1 + 1000000LL) : LLONG_MAX;
    if (upper < lower) upper = lower;
    uniform_int_distribution<long long> dist(lower, upper);
    return dist(gen);
}

int main() {
    vector<long long> tests = {1000, 1001, 50000, 0, 123456789};
    for (auto m : tests) {
        long long val = randomGreaterThan(m);
        cout << "Random > " << m << ": " << val << "\n";
    }
    return 0;
}