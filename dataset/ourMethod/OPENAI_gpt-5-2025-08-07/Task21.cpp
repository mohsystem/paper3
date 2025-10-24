#include <iostream>
#include <vector>
#include <limits>
#include <stdexcept>

std::vector<int> removeSmallest(const std::vector<int>& arr) {
    // Validate input size
    if (arr.empty()) {
        return {};
    }

    // Find index of the first smallest value
    size_t minIndex = 0;
    int minVal = arr[0];
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIndex = i;
        }
    }

    // Build result without mutating the original
    std::vector<int> result;
    result.reserve(arr.size() - 1);
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i != minIndex) {
            result.push_back(arr[i]);
        }
    }
    return result;
}

void printVector(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ",";
        std::cout << v[i];
    }
    std::cout << "]";
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> tests = {
        {1,2,3,4,5},
        {5,3,2,1,4},
        {2,2,1,2,1},
        {},          // empty
        {7}          // single element
    };

    for (size_t t = 0; t < tests.size(); ++t) {
        std::cout << "Input: ";
        printVector(tests[t]);
        std::vector<int> out = removeSmallest(tests[t]);
        std::cout << " -> Output: ";
        printVector(out);
        std::cout << "\n";
    }
    return 0;
}