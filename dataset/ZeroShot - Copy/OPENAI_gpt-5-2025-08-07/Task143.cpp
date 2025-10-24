#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

std::vector<int> sortAscending(const std::vector<int>& input) {
    std::vector<int> copy = input;
    std::sort(copy.begin(), copy.end(), [](int a, int b){ return a < b; });
    return copy;
}

void printArray(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << arr[i];
    }
    std::cout << "]\n";
}

int main() {
    // Test case 1
    std::vector<int> t1 = {5, 2, 9, 1, 5, 6};
    printArray(sortAscending(t1));

    // Test case 2
    std::vector<int> t2 = {-3, 0, -1, -7, 2};
    printArray(sortAscending(t2));

    // Test case 3
    std::vector<int> t3 = {};
    printArray(sortAscending(t3));

    // Test case 4
    std::vector<int> t4 = {42};
    printArray(sortAscending(t4));

    // Test case 5
    std::vector<int> t5 = {INT_MAX, INT_MIN, 0, -1, 1};
    printArray(sortAscending(t5));

    return 0;
}