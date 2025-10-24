#include <iostream>
#include <vector>
#include <algorithm>

/**
 * @brief Calculates the length of the longest consecutive-run in a vector.
 * A consecutive-run is a list of adjacent, consecutive integers,
 * which can be either increasing or decreasing.
 * 
 * @param arr The input vector of integers.
 * @return The length of the longest consecutive-run.
 */
int longestRun(const std::vector<int>& arr) {
    if (arr.empty()) {
        return 0;
    }
    if (arr.size() == 1) {
        return 1;
    }

    int maxLength = 1;
    int currentIncreasingLength = 1;
    int currentDecreasingLength = 1;

    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] == arr[i - 1] + 1) {
            currentIncreasingLength++;
            currentDecreasingLength = 1;
        } else if (arr[i] == arr[i - 1] - 1) {
            currentDecreasingLength++;
            currentIncreasingLength = 1;
        } else {
            currentIncreasingLength = 1;
            currentDecreasingLength = 1;
        }
        maxLength = std::max({maxLength, currentIncreasingLength, currentDecreasingLength});
    }
    return maxLength;
}

int main() {
    // Test cases
    std::vector<int> test1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "longestRun([1, 2, 3, 5, 6, 7, 8, 9]) -> " << longestRun(test1) << std::endl;

    std::vector<int> test2 = {1, 2, 3, 10, 11, 15};
    std::cout << "longestRun([1, 2, 3, 10, 11, 15]) -> " << longestRun(test2) << std::endl;

    std::vector<int> test3 = {5, 4, 2, 1};
    std::cout << "longestRun([5, 4, 2, 1]) -> " << longestRun(test3) << std::endl;

    std::vector<int> test4 = {3, 5, 7, 10, 15};
    std::cout << "longestRun([3, 5, 7, 10, 15]) -> " << longestRun(test4) << std::endl;
    
    std::vector<int> test5 = {1, 2, 3, 2, 1, 0, 1, 2};
    std::cout << "longestRun([1, 2, 3, 2, 1, 0, 1, 2]) -> " << longestRun(test5) << std::endl;

    return 0;
}