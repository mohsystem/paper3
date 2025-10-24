#include <iostream>
#include <vector>
#include <algorithm> // For std::max

/**
 * @brief Calculates the length of the longest consecutive-run in a vector of numbers.
 * A consecutive-run is a list of adjacent, consecutive integers, either increasing or decreasing.
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

    int maxRun = 1;
    int currentRun = 1;
    // direction: 0 = undecided, 1 = increasing, -1 = decreasing
    int direction = 0;

    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] == arr[i - 1] + 1) { // Increasing
            if (direction == 1) {
                currentRun++;
            } else {
                currentRun = 2;
                direction = 1;
            }
        } else if (arr[i] == arr[i - 1] - 1) { // Decreasing
            if (direction == -1) {
                currentRun++;
            } else {
                currentRun = 2;
                direction = -1;
            }
        } else { // Run broken
            currentRun = 1;
            direction = 0;
        }
        maxRun = std::max(maxRun, currentRun);
    }
    return maxRun;
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test 1: longestRun({1, 2, 3, 5, 6, 7, 8, 9})" << std::endl;
    std::cout << "Expected: 5, Got: " << longestRun(arr1) << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {1, 2, 3, 10, 11, 15};
    std::cout << "\nTest 2: longestRun({1, 2, 3, 10, 11, 15})" << std::endl;
    std::cout << "Expected: 3, Got: " << longestRun(arr2) << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {5, 4, 2, 1};
    std::cout << "\nTest 3: longestRun({5, 4, 2, 1})" << std::endl;
    std::cout << "Expected: 2, Got: " << longestRun(arr3) << std::endl;

    // Test Case 4
    std::vector<int> arr4 = {3, 5, 7, 10, 15};
    std::cout << "\nTest 4: longestRun({3, 5, 7, 10, 15})" << std::endl;
    std::cout << "Expected: 1, Got: " << longestRun(arr4) << std::endl;

    // Test Case 5
    std::vector<int> arr5 = {1, 0, -1, -2, 5, 6, 5, 4, 3, 2, 1};
    std::cout << "\nTest 5: longestRun({1, 0, -1, -2, 5, 6, 5, 4, 3, 2, 1})" << std::endl;
    std::cout << "Expected: 6, Got: " << longestRun(arr5) << std::endl;

    return 0;
}