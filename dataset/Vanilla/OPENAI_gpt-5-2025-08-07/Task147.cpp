#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

int kthLargest(const std::vector<int>& arr, int k) {
    if (k < 1 || k > static_cast<int>(arr.size())) {
        throw std::invalid_argument("Invalid k or array");
    }
    std::vector<int> copy = arr;
    std::sort(copy.begin(), copy.end());
    return copy[copy.size() - k];
}

int main() {
    std::vector<std::vector<int>> tests = {
        {3, 2, 1, 5, 6, 4},
        {3, 2, 3, 1, 2, 4, 5, 5, 6},
        {7},
        {-1, -2, -3, -4},
        {5, 5, 5, 5}
    };
    std::vector<int> ks = {2, 4, 1, 2, 3};

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << kthLargest(tests[i], ks[i]) << std::endl;
    }
    return 0;
}