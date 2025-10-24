#include <iostream>
#include <vector>
#include <algorithm>

std::vector<int> posNegSort(const std::vector<int>& arr) {
    std::vector<int> result(arr.size());
    std::vector<int> positives;
    positives.reserve(arr.size());
    for (int v : arr) {
        if (v > 0) positives.push_back(v);
    }
    std::sort(positives.begin(), positives.end());
    size_t idx = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] < 0) {
            result[i] = arr[i];
        } else {
            result[i] = positives[idx++];
        }
    }
    return result;
}

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    std::vector<std::vector<int>> tests = {
        {6, 3, -2, 5, -8, 2, -2},
        {6, 5, 4, -1, 3, 2, -1, 1},
        {-5, -5, -5, -5, 7, -5},
        {},
        {5, -1, -2, 4, 3}
    };
    for (const auto& t : tests) {
        printVec(posNegSort(t));
    }
    return 0;
}