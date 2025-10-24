#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

std::vector<int> posNegSort(const std::vector<int>& arr) {
    if (arr.empty()) return {};

    std::vector<int> positives;
    positives.reserve(arr.size());
    for (int v : arr) {
        if (v > 0) positives.push_back(v);
    }
    std::sort(positives.begin(), positives.end());

    std::vector<int> result;
    result.reserve(arr.size());
    std::size_t p = 0;
    for (int v : arr) {
        if (v < 0) {
            result.push_back(v);
        } else {
            result.push_back(positives[p++]);
        }
    }
    return result;
}

void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]";
}

int main() {
    std::vector<std::vector<int>> tests = {
        {6, 3, -2, 5, -8, 2, -2},
        {6, 5, 4, -1, 3, 2, -1, 1},
        {-5, -5, -5, -5, 7, -5},
        {},
        {-1, -3, 2, 1, 4}
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " input:  ";
        printVec(tests[i]);
        std::cout << "\nTest " << (i + 1) << " output: ";
        auto out = posNegSort(tests[i]);
        printVec(out);
        std::cout << "\n\n";
    }
    return 0;
}