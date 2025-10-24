#include <iostream>
#include <vector>
#include <numeric>

/**
 * Finds an index in a vector of integers where the sum of integers to the left
 * of the index is equal to the sum of integers to the right.
 *
 * @param arr The input vector of integers.
 * @return The lowest index N where the condition is met, or -1 if no such index exists.
 */
int findEvenIndex(const std::vector<int>& arr) {
    // Use long long for sums to prevent potential integer overflow, a security best practice.
    long long totalSum = 0;
    for (int num : arr) {
        totalSum += num;
    }
    
    long long leftSum = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        // The sum of the right side is the total sum minus the left sum and the current element.
        long long rightSum = totalSum - leftSum - arr[i];

        if (leftSum == rightSum) {
            return static_cast<int>(i);
        }
        
        leftSum += arr[i];
    }
    
    return -1; // Return -1 if no such index is found.
}

// Helper function to print a vector and run a test case
void runTestCase(const std::string& name, const std::vector<int>& testArr) {
    std::cout << name << " Array: {";
    for (size_t i = 0; i < testArr.size(); ++i) {
        std::cout << testArr[i] << (i == testArr.size() - 1 ? "" : ", ");
    }
    std::cout << "} -> Index: " << findEvenIndex(testArr) << std::endl;
}

int main() {
    // Test Case 1: Example from prompt
    runTestCase("Test Case 1:", {1, 2, 3, 4, 3, 2, 1});

    // Test Case 2: Example from prompt
    runTestCase("Test Case 2:", {1, 100, 50, -51, 1, 1});

    // Test Case 3: Example from prompt (index 0)
    runTestCase("Test Case 3:", {20, 10, -80, 10, 10, 15, 35});
    
    // Test Case 4: Index is the last element
    runTestCase("Test Case 4:", {10, -80, 10, 10, 15, 35, 20});

    // Test Case 5: No solution
    runTestCase("Test Case 5:", {1, 2, 3, 4, 5, 6});

    return 0;
}