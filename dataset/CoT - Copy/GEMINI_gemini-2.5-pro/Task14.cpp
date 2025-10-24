#include <iostream>
#include <vector>
#include <numeric>

/**
 * Finds an index N in a vector of integers where the sum of the integers
 * to the left of N is equal to the sum of the integers to the right of N.
 *
 * @param arr The input vector of integers.
 * @return The lowest index N that satisfies the condition, or -1 if no such index exists.
 */
int findEvenIndex(const std::vector<int>& arr) {
    // Use long long to prevent potential integer overflow with large array values
    long long totalSum = std::accumulate(arr.begin(), arr.end(), 0LL);
    
    long long leftSum = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        // The right sum is the total sum minus the left sum and the current element
        long long rightSum = totalSum - leftSum - arr[i];
        
        if (leftSum == rightSum) {
            return i; // Found the equilibrium index
        }
        
        // Update the left sum for the next iteration
        leftSum += arr[i];
    }
    
    return -1; // No such index found
}

void printVector(const std::vector<int>& arr) {
    std::cout << "Array: [";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1
    std::vector<int> test1 = {1, 2, 3, 4, 3, 2, 1};
    printVector(test1);
    std::cout << "Expected: 3, Actual: " << findEvenIndex(test1) << std::endl << std::endl;

    // Test Case 2
    std::vector<int> test2 = {1, 100, 50, -51, 1, 1};
    printVector(test2);
    std::cout << "Expected: 1, Actual: " << findEvenIndex(test2) << std::endl << std::endl;

    // Test Case 3
    std::vector<int> test3 = {20, 10, -80, 10, 10, 15, 35};
    printVector(test3);
    std::cout << "Expected: 0, Actual: " << findEvenIndex(test3) << std::endl << std::endl;

    // Test Case 4
    std::vector<int> test4 = {10, -80, 10, 10, 15, 35, 20};
    printVector(test4);
    std::cout << "Expected: 6, Actual: " << findEvenIndex(test4) << std::endl << std::endl;

    // Test Case 5 (No solution)
    std::vector<int> test5 = {1, 2, 3, 4, 5, 6};
    printVector(test5);
    std::cout << "Expected: -1, Actual: " << findEvenIndex(test5) << std::endl << std::endl;

    return 0;
}