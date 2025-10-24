#include <iostream>
#include <vector>
#include <string>

/**
 * Finds the outlier integer in an array that is otherwise comprised of either all even or all odd integers.
 *
 * @param integers The input vector of integers (size will be at least 3).
 * @return The outlier integer N.
 */
int findOutlier(const std::vector<int>& integers) {
    // According to the prompt, the size is at least 3, so direct access is safe.
    
    int evenCount = 0;
    // Check the first three elements to determine the majority parity.
    for (int i = 0; i < 3; ++i) {
        if (integers[i] % 2 == 0) {
            evenCount++;
        }
    }

    bool majorityIsEven = (evenCount >= 2);

    // Iterate through the vector to find the single number with the opposite parity.
    for (int num : integers) {
        if (majorityIsEven) {
            // If the majority is even, we are looking for the single odd number.
            if (num % 2 != 0) {
                return num;
            }
        } else {
            // If the majority is odd, we are looking for the single even number.
            if (num % 2 == 0) {
                return num;
            }
        }
    }
    
    // This part should be unreachable given the problem constraints.
    return 0; 
}

void runTest(const std::string& testName, const std::vector<int>& testData) {
    std::cout << testName << ": [";
    for(size_t i = 0; i < testData.size(); ++i) {
        std::cout << testData[i] << (i == testData.size() - 1 ? "" : ", ");
    }
    std::cout << "] -> " << findOutlier(testData) << std::endl;
}

int main() {
    // Test cases
    runTest("Test 1", {2, 4, 0, 100, 4, 11, 2602, 36});
    runTest("Test 2", {160, 3, 1719, 19, 11, 13, -21});
    runTest("Test 3", {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2});
    runTest("Test 4", {2, 2, 2, 2, 3, 2, 2, 2});
    runTest("Test 5", {1, -3, 5, 7, 9, -12, 11, 13});
    
    return 0;
}