#include <iostream>
#include <vector>
#include <queue>
#include <functional>

int queueTime(const std::vector<int>& customers, int n) {
    if (n <= 0) throw std::invalid_argument("n must be positive");
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (int i = 0; i < n; ++i) pq.push(0);
    for (int t : customers) {
        int next = pq.top(); pq.pop();
        pq.push(next + t);
    }
    int total = 0;
    while (!pq.empty()) {
        total = std::max(total, pq.top());
        pq.pop();
    }
    return total;
}

int main() {
    std::vector<std::pair<std::vector<int>, int>> tests = {
        {{5, 3, 4}, 1},
        {{10, 2, 3, 3}, 2},
        {{2, 3, 10}, 2},
        {{}, 1},
        {{1, 2, 3, 4, 5}, 100}
    };
    for (auto& test : tests) {
        std::cout << queueTime(test.first, test.second) << std::endl;
    }
    return 0;
}