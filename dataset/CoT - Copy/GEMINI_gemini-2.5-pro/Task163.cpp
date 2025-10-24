#include <iostream>
#include <vector>
#include <algorithm>

/**
 * Calculates the length of the longest consecutive run in a vector.
 * A consecutive run can be either increasing or decreasing.
 *
 * @param arr The input vector of integers.
 * @return The length of the longest consecutive run.
 */
int longestRun(const std::vector<int>& arr) {
    if (arr.empty()) {
        return 0;
    }
    if (arr.size() == 1) {
        return 1;
    }

    int maxLength = 1;
    int currentIncreasing = 1;
    int currentDecreasing = 1;

    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] == arr[i - 1] + 1) {
            // Increasing run continues
            currentIncreasing++;
            // Reset decreasing run
            currentDecreasing = 1;
        } else if (arr[i] == arr[i - 1] - 1) {
            // Decreasing run continues
            currentDecreasing++;
            // Reset increasing run
            currentIncreasing = 1;
        } else {
            // Run is broken
            currentIncreasing = 1;
            currentDecreasing = 1;
        }
        maxLength = std::max({maxLength, currentIncreasing, currentDecreasing});
    }
    return maxLength;
}

void printVector(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}


int main() {
    // Test Case 1
    std::vector<int> arr1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test Case 1: ";
    printVector(arr1);
    std::cout << " -> " << longestRun(arr1) << std::endl; // Expected: 5

    // Test Case 2
    std::vector<int> arr2 = {1, 2, 3, 10, 11, 15};
    std::cout << "Test Case 2: ";
    printVector(arr2);
    std::cout << " -> " << longestRun(arr2) << std::endl; // Expected: 3

    // Test Case 3
    std::vector<int> arr3 = {5, 4, 2, 1};
    std::cout << "Test Case 3: ";
    printVector(arr3);
    std::cout << " -> " << longestRun(arr3) << std::endl; // Expected: 2

    // Test Case 4
    std::vector<int> arr4 = {3, 5, 7, 10, 15};
    std::cout << "Test Case 4: ";
    printVector(arr4);
    std::cout << " -> " << longestRun(arr4) << std::endl; // Expected: 1

    // Test Case 5
    std::vector<int> arr5 = {1, 0, -1, -2, -1, 0, 1, 2, 3};
    std::cout << "Test Case 5: ";
    printVector(arr5);
    std::cout << " -> " << longestRun(arr5) << std::endl; // Expected: 6

    return 0;
}