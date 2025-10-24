#include <iostream>
#include <vector>

std::vector<int> removeSmallest(const std::vector<int>& arr) {
    if (arr.empty()) return {};
    int minIdx = 0;
    int minVal = arr[0];
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIdx = static_cast<int>(i);
        }
    }
    std::vector<int> result;
    result.reserve(arr.size() > 0 ? arr.size() - 1 : 0);
    for (size_t i = 0; i < arr.size(); ++i) {
        if (static_cast<int>(i) == minIdx) continue;
        result.push_back(arr[i]);
    }
    return result;
}

void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1, 2, 3, 4, 5},
        {5, 3, 2, 1, 4},
        {2, 2, 1, 2, 1},
        {},
        {7}
    };
    for (const auto& t : tests) {
        auto res = removeSmallest(t);
        printVec(res);
    }
    return 0;
}