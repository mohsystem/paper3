#include <iostream>
#include <vector>
#include <cstddef>
#include <cstdint>

int findEvenIndex(const std::vector<int>& arr) {
    if (arr.empty()) return -1;
    long long total = 0;
    for (int v : arr) total += static_cast<long long>(v);
    long long left = 0;
    for (std::size_t i = 0; i < arr.size(); ++i) {
        long long right = total - left - static_cast<long long>(arr[i]);
        if (left == right) return static_cast<int>(i);
        left += static_cast<long long>(arr[i]);
    }
    return -1;
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1,2,3,4,3,2,1},           // expect 3
        {1,100,50,-51,1,1},        // expect 1
        {20,10,-80,10,10,15,35},   // expect 0
        {1,2,3,4,5},               // expect -1
        {0,0,0}                    // expect 0
    };
    for (std::size_t i = 0; i < tests.size(); ++i) {
        int res = findEvenIndex(tests[i]);
        std::cout << "Test " << (i+1) << " -> " << res << "\n";
    }
    return 0;
}