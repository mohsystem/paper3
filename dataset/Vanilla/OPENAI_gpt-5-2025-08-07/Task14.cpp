#include <iostream>
#include <vector>
#include <numeric>

int findEvenIndex(const std::vector<int>& arr) {
    long long total = std::accumulate(arr.begin(), arr.end(), 0LL);
    long long left = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        long long right = total - left - arr[i];
        if (left == right) return static_cast<int>(i);
        left += arr[i];
    }
    return -1;
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1, 2, 3, 4, 3, 2, 1},
        {1, 100, 50, -51, 1, 1},
        {20, 10, -80, 10, 10, 15, 35},
        {1, 2, 3, 4, 5, 6},
        {0, 0, 0, 0}
    };
    for (const auto& t : tests) {
        std::cout << "[";
        for (size_t i = 0; i < t.size(); ++i) {
            std::cout << t[i] << (i + 1 < t.size() ? ", " : "");
        }
        std::cout << "] -> " << findEvenIndex(t) << "\n";
    }
    return 0;
}