#include <iostream>
#include <vector>
#include <limits>

int findEvenIndex(const std::vector<long long>& arr) {
    if (arr.empty()) {
        return -1;
    }
    long long total = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        total += arr[i];
    }
    long long left = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        long long right = total - left - arr[i];
        if (left == right) {
            return static_cast<int>(i);
        }
        left += arr[i];
    }
    return -1;
}

int main() {
    // 5 test cases
    std::vector<std::vector<long long>> tests = {
        {1, 2, 3, 4, 3, 2, 1},          // Expected 3
        {1, 100, 50, -51, 1, 1},        // Expected 1
        {20, 10, -80, 10, 10, 15, 35},  // Expected 0
        {1, 2, 3, 4, 5, 6},             // Expected -1
        {0, 0, 0, 0}                     // Expected 0
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        int idx = findEvenIndex(tests[i]);
        std::cout << "Test " << (i + 1) << " result: " << idx << std::endl;
    }

    return 0;
}