#include <iostream>
#include <vector>
#include <string>
#include <numeric> // For std::accumulate

/**
 * @brief Given a list of integers, determine whether the sum of its elements is odd or even.
 * 
 * @param arr A constant reference to a vector of integers. An empty vector is considered as [0].
 * @return A string, "even" or "odd".
 */
std::string oddOrEven(const std::vector<int>& arr) {
    // Use long long for the sum to prevent potential integer overflow.
    // std::accumulate is a safe and efficient way to sum elements.
    // The initial value 0LL ensures the accumulator is a long long.
    long long sum = std::accumulate(arr.begin(), arr.end(), 0LL);

    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

// Helper function to print a vector and run a test case
void run_test_case(const std::vector<int>& test_arr) {
    std::cout << "Input: [";
    for (size_t i = 0; i < test_arr.size(); ++i) {
        std::cout << test_arr[i] << (i == test_arr.size() - 1 ? "" : ", ");
    }
    std::cout << "], Output: " << oddOrEven(test_arr) << std::endl;
}

int main() {
    // Test Case 1: Standard even case
    run_test_case({0});

    // Test Case 2: Standard odd case
    run_test_case({0, 1, 4});

    // Test Case 3: Case with negative numbers
    run_test_case({0, -1, -5});

    // Test Case 4: Empty vector
    run_test_case({});

    // Test Case 5: A longer list
    run_test_case({1, 2, 3, 4, 5, 6, 7, 8, 9});
    
    return 0;
}