#include <iostream>
#include <vector>

/**
 * Finds the outlier number in a vector where all numbers are either even or odd, except for one.
 *
 * @param arr The input vector of integers (size >= 3).
 * @return The single outlier integer.
 */
int findOutlier(const std::vector<int>& arr) {
    int oddCount = 0;
    int evenCount = 0;
    int lastOdd = 0;
    int lastEven = 0;

    for (int n : arr) {
        if (n % 2 == 0) {
            evenCount++;
            lastEven = n;
        } else {
            oddCount++;
            lastOdd = n;
        }
    }

    if (evenCount == 1) {
        return lastEven;
    } else {
        return lastOdd;
    }
}

int main() {
    // Test Case 1: Outlier is odd
    std::vector<int> test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
    std::cout << "Test 1 Outlier: " << findOutlier(test1) << std::endl;

    // Test Case 2: Outlier is even
    std::vector<int> test2 = {160, 3, 1719, 19, 11, 13, -21};
    std::cout << "Test 2 Outlier: " << findOutlier(test2) << std::endl;

    // Test Case 3: Small array, outlier is even
    std::vector<int> test3 = {1, 1, 0, 1, 1};
    std::cout << "Test 3 Outlier: " << findOutlier(test3) << std::endl;

    // Test Case 4: Outlier is odd, at the end
    std::vector<int> test4 = {2, 6, 8, -10, 3};
    std::cout << "Test 4 Outlier: " << findOutlier(test4) << std::endl;
    
    // Test Case 5: Negative numbers, outlier is even
    std::vector<int> test5 = {-3, -5, -7, -9, 10};
    std::cout << "Test 5 Outlier: " << findOutlier(test5) << std::endl;

    return 0;
}