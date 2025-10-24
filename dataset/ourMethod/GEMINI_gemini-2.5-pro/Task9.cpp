#include <iostream>
#include <vector>
#include <string>
#include <numeric>

// Function to determine if the sum of elements is odd or even.
// Takes a constant reference to a vector of integers.
// Returns a std::string ("odd" or "even").
std::string oddOrEven(const std::vector<int>& arr) {
    // The problem states to treat an empty array as [0].
    // A loop over an empty vector will result in a sum of 0, which is correct.
    long long sum = 0; // Use long long to prevent potential integer overflow.
    
    // Iterate through each number in the vector and add it to the sum.
    for (int num : arr) {
        sum += num;
    }

    // Check the parity of the sum.
    // The modulo operator (%) correctly handles negative numbers for this parity check.
    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

int main() {
    // Test Case 1: [0] -> Expected: "even"
    std::vector<int> test1 = {0};
    std::cout << "Test 1: Input: [0], Output: " << oddOrEven(test1) << std::endl;

    // Test Case 2: [0, 1, 4] -> Expected: "odd"
    std::vector<int> test2 = {0, 1, 4};
    std::cout << "Test 2: Input: [0, 1, 4], Output: " << oddOrEven(test2) << std::endl;

    // Test Case 3: [0, -1, -5] -> Expected: "even"
    std::vector<int> test3 = {0, -1, -5};
    std::cout << "Test 3: Input: [0, -1, -5], Output: " << oddOrEven(test3) << std::endl;

    // Test Case 4: [] -> Expected: "even" (empty array is treated as [0])
    std::vector<int> test4 = {};
    std::cout << "Test 4: Input: [], Output: " << oddOrEven(test4) << std::endl;

    // Test Case 5: [1, 2, 3, 4, 5] -> Expected: "odd"
    std::vector<int> test5 = {1, 2, 3, 4, 5};
    std::cout << "Test 5: Input: [1, 2, 3, 4, 5], Output: " << oddOrEven(test5) << std::endl;

    return 0;
}