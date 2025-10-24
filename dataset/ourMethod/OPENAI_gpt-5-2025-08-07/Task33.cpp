#include <bits/stdc++.h>
using namespace std;

long long queueTime(const vector<int>& customers, int n) {
    if (n <= 0) return -1; // invalid number of tills
    for (size_t i = 0; i < customers.size(); ++i) {
        if (customers[i] < 0) return -1; // invalid customer time
    }
    if (customers.empty()) return 0;

    size_t m = static_cast<size_t>(n);
    if (m > customers.size()) m = customers.size();

    priority_queue<long long, vector<long long>, greater<long long>> pq;
    // Initialize tills
    for (size_t i = 0; i < m; ++i) {
        pq.push(static_cast<long long>(customers[i]));
    }
    // Assign remaining customers to the earliest available till
    for (size_t i = m; i < customers.size(); ++i) {
        long long t = pq.top(); pq.pop();
        if (customers[i] > LLONG_MAX - t) return -1; // overflow check
        pq.push(t + static_cast<long long>(customers[i]));
    }
    // The total time is the maximum finish time among tills
    long long total = 0;
    while (!pq.empty()) {
        total = max(total, pq.top());
        pq.pop();
    }
    return total;
}

int main() {
    vector<pair<vector<int>, int>> tests = {
        {{5,3,4}, 1},
        {{10,2,3,3}, 2},
        {{2,3,10}, 2},
        {{}, 1},
        {{1,2,3,4,5}, 100}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        long long res = queueTime(tests[i].first, tests[i].second);
        cout << "Test " << (i+1) << ": " << res << "\n";
    }
    return 0;
}