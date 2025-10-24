#include <iostream>
#include <vector>
#include <algorithm> // For std::max

/**
 * @brief Finds the length of the longest consecutive run in a vector of numbers.
 * A consecutive run can be either increasing or decreasing.
 * 
 * @param arr The input vector of integers.
 * @return The length of the longest consecutive run.
 */
int longestRun(const std::vector<int>& arr) {
    if (arr.empty()) {
        return 0;
    }

    size_t n = arr.size();
    if (n == 1) {
        return 1;
    }

    int maxLength = 1;
    int currentIncLength = 1;
    int currentDecLength = 1;

    for (size_t i = 1; i < n; ++i) {
        // Check for an increasing run
        if (arr[i] == arr[i - 1] + 1) {
            currentIncLength++;
        } else {
            currentIncLength = 1;
        }

        // Check for a decreasing run
        if (arr[i] == arr[i - 1] - 1) {
            currentDecLength++;
        } else {
            currentDecLength = 1;
        }

        // Update the maximum length found so far
        maxLength = std::max({maxLength, currentIncLength, currentDecLength});
    }

    return maxLength;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "{";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "}";
}

int main() {
    // Test Case 1: Long increasing run
    std::vector<int> test1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test 1: ";
    printVector(test1);
    std::cout << " -> Expected: 5, Got: " << longestRun(test1) << std::endl;

    // Test Case 2: Long decreasing run
    std::vector<int> test2 = {9, 8, 7, 6, 3, 2, 1};
    std::cout << "Test 2: ";
    printVector(test2);
    std::cout << " -> Expected: 4, Got: " << longestRun(test2) << std::endl;

    // Test Case 3: Mixed runs
    std::vector<int> test3 = {1, 2, 1, 0, -1, 4, 5};
    std::cout << "Test 3: ";
    printVector(test3);
    std::cout << " -> Expected: 4, Got: " << longestRun(test3) << std::endl;

    // Test Case 4: No consecutive runs
    std::vector<int> test4 = {3, 5, 7, 10, 15};
    std::cout << "Test 4: ";
    printVector(test4);
    std::cout << " -> Expected: 1, Got: " << longestRun(test4) << std::endl;

    // Test Case 5: Empty vector
    std::vector<int> test5 = {};
    std::cout << "Test 5: ";
    printVector(test5);
    std::cout << " -> Expected: 0, Got: " << longestRun(test5) << std::endl;

    return 0;
}