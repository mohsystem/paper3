#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

/**
 * Sorts the positive numbers in a vector in ascending order,
 * while keeping the negative numbers in their original positions.
 *
 * @param arr The input vector of integers.
 * @return A new vector with positive numbers sorted.
 */
std::vector<int> posNegSort(const std::vector<int>& arr) {
    std::vector<int> positives;
    for (int num : arr) {
        if (num > 0) {
            positives.push_back(num);
        }
    }

    std::sort(positives.begin(), positives.end());

    std::vector<int> result;
    if (arr.empty()) {
        return result;
    }
    
    result.reserve(arr.size());
    auto pos_it = positives.begin();
    for (int num : arr) {
        if (num > 0) {
            result.push_back(*pos_it);
            ++pos_it;
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
    std::vector<std::vector<int>> testCases = {
        {6, 3, -2, 5, -8, 2, -2},
        {6, 5, 4, -1, 3, 2, -1, 1},
        {-5, -5, -5, -5, 7, -5},
        {},
        {1, -1, 2, -2, 3, -3}
    };

    for (const auto& testCase : testCases) {
        std::cout << "Original: ";
        printVector(testCase);
        std::cout << std::endl;
        
        std::vector<int> sorted = posNegSort(testCase);
        
        std::cout << "Sorted:   ";
        printVector(sorted);
        std::cout << std::endl << std::endl;
    }

    return 0;
}