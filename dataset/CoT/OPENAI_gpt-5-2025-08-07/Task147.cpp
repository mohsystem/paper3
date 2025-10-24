#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <stdexcept>

// Step 1 & 2: Understand and secure - validate inputs
int findKthLargest(const std::vector<int>& nums, int k) {
    if (k < 1 || static_cast<size_t>(k) > nums.size()) {
        throw std::invalid_argument("k must be between 1 and the length of the array.");
    }

    // Step 3: Secure coding - use min-heap of size k
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
    for (int num : nums) {
        if (static_cast<int>(minHeap.size()) < k) {
            minHeap.push(num);
        } else if (num > minHeap.top()) {
            minHeap.pop();
            minHeap.push(num);
        }
    }
    // Step 4: minHeap contains k largest; top is kth largest
    return minHeap.top();
}

// Step 5: Main with 5 test cases
int main() {
    try {
        std::vector<std::pair<std::vector<int>, int>> tests = {
            {{3, 2, 1, 5, 6, 4}, 2},
            {{3, 2, 3, 1, 2, 4, 5, 5, 6}, 4},
            {{-1, -3, -2, -4, -5}, 2},
            {{7}, 1},
            {{9, 9, 8, 7, 6, 5}, 3}
        };
        for (size_t i = 0; i < tests.size(); ++i) {
            int res = findKthLargest(tests[i].first, tests[i].second);
            std::cout << "Test " << (i + 1) << " -> kth largest: " << res << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}