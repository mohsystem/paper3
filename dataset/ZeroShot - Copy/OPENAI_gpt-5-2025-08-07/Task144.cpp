#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

std::vector<int> mergeSorted(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() > std::numeric_limits<size_t>::max() - b.size()) {
        throw std::overflow_error("Merged size overflow");
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

void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    try {
        std::vector<int> a1 = {};
        std::vector<int> b1 = {};
        printVec(mergeSorted(a1, b1));

        std::vector<int> a2 = {1, 3, 5};
        std::vector<int> b2 = {2, 4, 6};
        printVec(mergeSorted(a2, b2));

        std::vector<int> a3 = {};
        std::vector<int> b3 = {7, 8};
        printVec(mergeSorted(a3, b3));

        std::vector<int> a4 = {-5, -1, 0, 2};
        std::vector<int> b4 = {-4, -3, 3, 10};
        printVec(mergeSorted(a4, b4));

        std::vector<int> a5 = {1, 2, 2, 3};
        std::vector<int> b5 = {2, 2, 4, 5};
        printVec(mergeSorted(a5, b5));
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}