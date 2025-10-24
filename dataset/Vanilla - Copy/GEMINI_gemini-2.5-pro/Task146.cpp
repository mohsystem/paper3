#include <iostream>
#include <vector>
#include <numeric> // For std::accumulate

int findMissingNumber(const std::vector<int>& arr) {
    // The vector is supposed to have numbers from 1 to n, but has n-1 elements.
    // So, n is the size of the vector + 1.
    long long n = arr.size() + 1;

    // Calculate the expected sum of the first n natural numbers.
    long long expectedSum = n * (n + 1) / 2;

    // Calculate the actual sum of elements in the vector.
    // Initialize accumulator with 0LL (long long zero) to avoid overflow.
    long long actualSum = std::accumulate(arr.begin(), arr.end(), 0LL);

    // The difference is the missing number.
    return static_cast<int>(expectedSum - actualSum);
}

int main() {
    // Test Case 1
    std::vector<int> arr1 = {1, 2, 4, 5};
    std::cout << "Test Case 1: Missing number in [1, 2, 4, 5] is " << findMissingNumber(arr1) << std::endl;

    // Test Case 2
    std::vector<int> arr2 = {2, 3, 1, 5};
    std::cout << "Test Case 2: Missing number in [2, 3, 1, 5] is " << findMissingNumber(arr2) << std::endl;

    // Test Case 3
    std::vector<int> arr3 = {1};
    std::cout << "Test Case 3: Missing number in [1] is " << findMissingNumber(arr3) << std::endl;

    // Test Case 4
    std::vector<int> arr4 = {1, 2, 3, 4, 5, 6, 8, 9};
    std::cout << "Test Case 4: Missing number in [1, 2, 3, 4, 5, 6, 8, 9] is " << findMissingNumber(arr4) << std::endl;
    
    // Test Case 5
    std::vector<int> arr5 = {2};
    std::cout << "Test Case 5: Missing number in [2] is " << findMissingNumber(arr5) << std::endl;

    return 0;
}