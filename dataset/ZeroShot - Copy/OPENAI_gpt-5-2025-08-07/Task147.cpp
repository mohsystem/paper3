#include <iostream>
#include <vector>
#include <queue>
#include <stdexcept>

int kthLargest(const std::vector<int>& nums, int k) {
    if (k < 1 || k > static_cast<int>(nums.size())) {
        throw std::invalid_argument("k must be between 1 and the length of the array.");
    }
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
    for (int v : nums) {
        if (static_cast<int>(minHeap.size()) < k) {
            minHeap.push(v);
        } else if (v > minHeap.top()) {
            minHeap.pop();
            minHeap.push(v);
        }
    }
    return minHeap.top();
}

int main() {
    std::vector<std::pair<std::vector<int>, int>> tests = {
        {{3, 2, 1, 5, 6, 4}, 2},
        {{3, 2, 3, 1, 2, 4, 5, 5, 6}, 4},
        {{-1, -1}, 2},
        {{7}, 1},
        {{3, 2, 1, 5, 6, 4}, 10} // invalid
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            int res = kthLargest(tests[i].first, tests[i].second);
            std::cout << "Test " << (i + 1) << " result: " << res << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (i + 1) << " error: " << ex.what() << "\n";
        }
    }
    return 0;
}