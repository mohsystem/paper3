#include <iostream>
#include <vector>
#include <algorithm> // for std::max

/**
 * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
 *
 * @param a The input vector of integers.
 * @return The sum of the maximum contiguous subarray. Returns 0 if the vector is empty.
 */
long long maxSubArraySum(const std::vector<int>& a) {
    // Secure: Handle empty vector to prevent out-of-bounds access.
    if (a.empty()) {
        return 0;
    }

    // Secure: Use long long for sums to prevent integer overflow.
    long long maxSoFar = a[0];
    long long currentMax = a[0];

    // Iterate from the second element
    for (size_t i = 1; i < a.size(); ++i) {
        // The maximum subarray ending at index i is either the element itself
        // or the element plus the maximum subarray ending at the previous index.
        currentMax = std::max((long long)a[i], currentMax + a[i]);
        
        // Update the overall maximum sum found so far.
        maxSoFar = std::max(maxSoFar, currentMax);
    }
    return maxSoFar;
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> testCases = {
        {-2, 1, -3, 4, -1, 2, 1, -5, 4}, // Expected: 6
        {1, 2, 3, 4, 5},                // Expected: 15
        {-2, -3, -4, -1, -5},           // Expected: -1
        {5},                            // Expected: 5
        {8, -19, 5, -4, 20}             // Expected: 21
    };

    for (int i = 0; i < testCases.size(); ++i) {
        const auto& arr = testCases[i];
        std::cout << "Test Case " << (i + 1) << ": Array = [";
        for (size_t j = 0; j < arr.size(); ++j) {
            std::cout << arr[j] << (j == arr.size() - 1 ? "" : ", ");
        }
        std::cout << "], Max Subarray Sum = " << maxSubArraySum(arr) << std::endl;
    }
    
    return 0;
}