#include <iostream>
#include <vector>
#include <numeric> // For std::accumulate
#include <string>

/**
 * Finds the missing number in a vector containing n-1 unique integers from 1 to n.
 * This function is secure against integer overflow for the sum calculation by using 'long long'.
 *
 * @param arr The input vector of unique integers. The vector is expected to contain
 *            unique numbers from 1 to n (inclusive) with one number missing.
 * @return The missing integer.
 */
int findMissingNumber(const std::vector<int>& arr) {
    // The complete list should contain 'n' numbers.
    // The vector has n-1 numbers, so n = arr.size() + 1.
    size_t n = arr.size() + 1;
    
    // Use 'long long' for the expected sum to prevent potential integer overflow if 'n' is large.
    long long expectedSum = static_cast<long long>(n) * (n + 1) / 2;
    
    // Use std::accumulate to sum vector elements. Initialize with 0LL (long long zero).
    long long actualSum = std::accumulate(arr.begin(), arr.end(), 0LL);
    
    // The difference is the missing number. Cast the result back to int.
    return static_cast<int>(expectedSum - actualSum);
}

// Helper function to run and print test cases
void run_test_case(const std::string& name, const std::vector<int>& arr) {
    std::cout << name << ": Array = {";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "}, Missing number: " << findMissingNumber(arr) << std::endl;
}

int main() {
    // Test Case 1: A standard case with a missing number in the middle.
    run_test_case("Test Case 1", {1, 2, 4, 5});

    // Test Case 2: An unordered array.
    run_test_case("Test Case 2", {5, 2, 1, 4});

    // Test Case 3: A small array where the last number is missing.
    run_test_case("Test Case 3", {1});
    
    // Test Case 4: An empty vector. Here n=1, so the missing number is 1.
    run_test_case("Test Case 4", {});

    // Test Case 5: A larger array.
    run_test_case("Test Case 5", {1, 2, 3, 4, 5, 6, 7, 9, 10});

    return 0;
}