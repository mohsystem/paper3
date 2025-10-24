#include <bits/stdc++.h>
using namespace std;

int queueTime(const vector<int>& customers, int n) {
    if (n <= 0 || customers.empty()) return 0;
    priority_queue<long long, vector<long long>, greater<long long>> pq;
    int tills = (int)min((size_t)n, customers.size());
    for (int i = 0; i < tills; ++i) {
        long long t = customers[i] < 0 ? 0 : customers[i];
        pq.push(t);
    }
    for (size_t i = tills; i < customers.size(); ++i) {
        long long t = pq.top(); pq.pop();
        long long c = customers[i] < 0 ? 0 : customers[i];
        pq.push(t + c);
    }
    long long last = 0;
    while (!pq.empty()) { last = pq.top(); pq.pop(); }
    if (last > INT_MAX) return INT_MAX;
    return (int)last;
}

int main() {
    // 5 test cases
    cout << queueTime({5,3,4}, 1) << "\n";           // 12
    cout << queueTime({10,2,3,3}, 2) << "\n";        // 10
    cout << queueTime({2,3,10}, 2) << "\n";          // 12
    cout << queueTime({}, 1) << "\n";                // 0
    cout << queueTime({1,2,3,4,5}, 100) << "\n";     // 5
    return 0;
}