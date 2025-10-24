#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

/**
 * Sorts the positive numbers in a vector in ascending order,
 * while keeping the negative numbers in their original positions.
 *
 * @param arr The input vector of integers (const reference).
 * @return A new vector with positive numbers sorted.
 */
std::vector<int> posNegSort(const std::vector<int>& arr) {
    // 1. Extract positive numbers into a separate vector.
    std::vector<int> positives;
    for (int num : arr) {
        if (num > 0) {
            positives.push_back(num);
        }
    }

    // 2. Sort the vector of positive numbers.
    std::sort(positives.begin(), positives.end());

    // 3. Create a new result vector and populate it.
    std::vector<int> result = arr;
    auto pos_iterator = positives.begin();
    for (int& num : result) {
        if (num > 0) {
            // Replace positive numbers with sorted ones.
            if (pos_iterator != positives.end()) {
                num = *pos_iterator;
                ++pos_iterator;
            }
        }
    }
    
    return result;
}

// Helper function to print a vector
void print_vector(const std::string& label, const std::vector<int>& vec) {
    std::cout << label;
    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<std::vector<int>> testCases = {
        {6, 3, -2, 5, -8, 2, -2},
        {6, 5, 4, -1, 3, 2, -1, 1},
        {-5, -5, -5, -5, 7, -5},
        {},
        {5, -1, 4, -2, 3, -3, 2, -4, 1}
    };

    int test_num = 1;
    for (const auto& testCase : testCases) {
        std::cout << "Test Case " << test_num++ << ":" << std::endl;
        print_vector("Original: ", testCase);
        std::vector<int> result = posNegSort(testCase);
        print_vector("Sorted:   ", result);
        std::cout << std::endl;
    }

    return 0;
}