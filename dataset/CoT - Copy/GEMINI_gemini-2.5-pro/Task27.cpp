#include <iostream>
#include <vector>

/**
 * Finds the outlier number in a vector where all numbers are either even or odd, except for one.
 *
 * @param arr The input vector of integers (size >= 3).
 * @return The outlier integer.
 */
int findOutlier(const std::vector<int>& arr) {
    // Since the vector has at least 3 elements, we can determine the majority
    // parity by checking the first three elements.
    int evenCount = 0;
    if ((arr[0] & 1) == 0) evenCount++;
    if ((arr[1] & 1) == 0) evenCount++;
    if ((arr[2] & 1) == 0) evenCount++;

    bool majorityIsEven = evenCount >= 2;

    // Iterate through the vector to find the number with the opposite parity.
    for (int num : arr) {
        if (majorityIsEven) {
            // If the majority is even, we are looking for the single odd number.
            if ((num & 1) != 0) {
                return num;
            }
        } else {
            // If the majority is odd, we are looking for the single even number.
            if ((num & 1) == 0) {
                return num;
            }
        }
    }
    
    // This part should be unreachable given the problem constraints.
    // It's included as a fallback.
    return 0;
}

void runTest(const std::string& testName, const std::vector<int>& testVec) {
    std::cout << testName << ": [";
    for(size_t i = 0; i < testVec.size(); ++i) {
        std::cout << testVec[i] << (i == testVec.size() - 1 ? "" : ", ");
    }
    std::cout << "] -> " << findOutlier(testVec) << std::endl;
}

int main() {
    // Test Case 1: Odd outlier
    std::vector<int> test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
    runTest("Test Case 1", test1);

    // Test Case 2: Even outlier
    std::vector<int> test2 = {160, 3, 1719, 19, 11, 13, -21};
    runTest("Test Case 2", test2);

    // Test Case 3: Even outlier at the beginning
    std::vector<int> test3 = {8, 1, 3, 5, 7, 9};
    runTest("Test Case 3", test3);

    // Test Case 4: Odd outlier at the end
    std::vector<int> test4 = {2, 4, 6, 8, 10, 11};
    runTest("Test Case 4", test4);
    
    // Test Case 5: Even outlier in the middle
    std::vector<int> test5 = {1, 3, 5, 2, 7, 9};
    runTest("Test Case 5", test5);

    return 0;
}