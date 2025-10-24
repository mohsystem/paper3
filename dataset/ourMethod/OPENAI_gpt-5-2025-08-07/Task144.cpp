#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

static bool isNonDecreasing(const std::vector<int>& v) {
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i - 1] > v[i]) return false;
    }
    return true;
}

std::vector<int> mergeSorted(const std::vector<int>& a, const std::vector<int>& b) {
    if (!isNonDecreasing(a) || !isNonDecreasing(b)) {
        throw std::invalid_argument("Input arrays must be sorted in non-decreasing order.");
    }
    if (a.size() > std::numeric_limits<size_t>::max() - b.size()) {
        throw std::invalid_argument("Combined array size is too large.");
    }

    std::vector<int> result;
    result.reserve(a.size() + b.size());
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] <= b[j]) {
            result.push_back(a[i++]);
        } else {
            result.push_back(b[j++]);
        }
    }
    while (i < a.size()) result.push_back(a[i++]);
    while (j < b.size()) result.push_back(b[j++]);

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
    try {
        // Test case 1: both empty
        std::vector<int> t1a = {};
        std::vector<int> t1b = {};
        printVec(mergeSorted(t1a, t1b));

        // Test case 2: one empty (b empty)
        std::vector<int> t2a = {1, 3, 5};
        std::vector<int> t2b = {};
        printVec(mergeSorted(t2a, t2b));

        // Test case 3: one empty (a empty)
        std::vector<int> t3a = {};
        std::vector<int> t3b = {2, 4, 6};
        printVec(mergeSorted(t3a, t3b));

        // Test case 4: duplicates
        std::vector<int> t4a = {1, 2, 2, 3};
        std::vector<int> t4b = {2, 2, 4};
        printVec(mergeSorted(t4a, t4b));

        // Test case 5: negatives and positives
        std::vector<int> t5a = {-5, -2, 0};
        std::vector<int> t5b = {-3, -1, 2};
        printVec(mergeSorted(t5a, t5b));
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}