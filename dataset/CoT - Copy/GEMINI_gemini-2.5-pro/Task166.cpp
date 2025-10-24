#include <iostream>
#include <vector>
#include <algorithm>

/**
 * Sorts the positive numbers in a vector in ascending order,
 * while keeping the negative numbers in their original positions.
 *
 * @param arr The input vector of integers.
 * @return A new vector with positive numbers sorted.
 */
std::vector<int> posNegSort(const std::vector<int>& arr) {
    // 1. Extract positive numbers
    std::vector<int> positives;
    for (int num : arr) {
        if (num > 0) {
            positives.push_back(num);
        }
    }

    // 2. Sort the positive numbers
    std::sort(positives.begin(), positives.end());

    // 3. Create the result vector by placing sorted positives back
    std::vector<int> result;
    result.reserve(arr.size());
    int posIndex = 0;
    for (int num : arr) {
        if (num > 0) {
            result.push_back(positives[posIndex++]);
        } else {
            result.push_back(num);
        }
    }
    return result;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> testCases = {
        {6, 3, -2, 5, -8, 2, -2},
        {6, 5, 4, -1, 3, 2, -1, 1},
        {-5, -5, -5, -5, 7, -5},
        {},
        {5, 4, 3, 2, 1}
    };

    for (const auto& testCase : testCases) {
        std::cout << "Original: ";
        printVector(testCase);
        std::cout << std::endl;
        
        std::vector<int> result = posNegSort(testCase);
        
        std::cout << "Sorted:   ";
        printVector(result);
        std::cout << std::endl << std::endl;
    }

    return 0;
}