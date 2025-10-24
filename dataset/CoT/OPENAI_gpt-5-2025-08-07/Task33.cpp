// Step 1-5 (Chain-of-Through) summarized:
// 1) Problem: Determine total checkout time given customers and n tills.
// 2) Security: Validate n, avoid unsafe behavior, handle empty inputs.
// 3) Implementation: Min-heap (priority_queue with greater) for till loads.
// 4) Review: Edge cases verified; standard library used safely.
// 5) Final: Returns integer; main includes 5 test cases.

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>

int queueTime(const std::vector<int>& customers, int n) {
    if (customers.empty()) return 0;
    if (n <= 0) throw std::invalid_argument("Number of tills must be positive.");
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (int i = 0; i < n; ++i) pq.push(0);
    for (int t : customers) {
        int earliest = pq.top();
        pq.pop();
        pq.push(earliest + t);
    }
    int total = 0;
    while (!pq.empty()) {
        total = std::max(total, pq.top());
        pq.pop();
    }
    return total;
}

int main() {
    std::vector<std::pair<std::vector<int>, int>> cases = {
        {{5, 3, 4}, 1},
        {{10, 2, 3, 3}, 2},
        {{2, 3, 10}, 2},
        {{}, 1},
        {{1, 2, 3, 4, 5}, 100}
    };
    std::vector<int> expected = {12, 10, 12, 0, 5};

    for (size_t i = 0; i < cases.size(); ++i) {
        int res = queueTime(cases[i].first, cases[i].second);
        std::cout << "Test " << (i + 1) << ": " << res << " (expected " << expected[i] << ")\n";
    }
    return 0;
}