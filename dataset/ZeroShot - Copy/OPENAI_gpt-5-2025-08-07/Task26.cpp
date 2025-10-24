#include <iostream>
#include <vector>
#include <stdexcept>

int findOdd(const std::vector<int>& arr) {
    if (arr.empty()) {
        throw std::invalid_argument("Input array must not be empty");
    }
    int x = 0;
    for (int v : arr) {
        x ^= v;
    }
    return x;
}

int main() {
    std::vector<std::vector<int>> tests = {
        {7},
        {0},
        {1, 1, 2},
        {0, 1, 0, 1, 0},
        {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            int res = findOdd(tests[i]);
            std::cout << "Test " << (i + 1) << " result: " << res << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Test " << (i + 1) << " error: " << e.what() << std::endl;
        }
    }
    return 0;
}