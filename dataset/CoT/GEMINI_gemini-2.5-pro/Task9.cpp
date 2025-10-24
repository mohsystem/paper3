#include <iostream>
#include <vector>
#include <string>
#include <numeric> // For std::accumulate

/**
 * @brief Given a vector of integers, determines if the sum of its elements is odd or even.
 * 
 * An empty vector is treated as [0].
 * 
 * @param arr The input vector of integers.
 * @return std::string "even" or "odd".
 */
std::string oddOrEven(const std::vector<int>& arr) {
    // std::accumulate with an empty range returns the initial value (0LL), which is even.
    // Using long long for the sum to prevent potential integer overflow.
    long long sum = std::accumulate(arr.begin(), arr.end(), 0LL);
    
    if (sum % 2 == 0) {
        return "even";
    } else {
        return "odd";
    }
}

void run_test_case(const std::vector<int>& test_case) {
    std::cout << "Input: [";
    for (size_t i = 0; i < test_case.size(); ++i) {
        std::cout << test_case[i] << (i == test_case.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    std::cout << "Output: " << oddOrEven(test_case) << std::endl;
}

int main() {
    // Test Case 1
    run_test_case({0}); // Expected: even

    // Test Case 2
    run_test_case({0, 1, 4}); // Expected: odd

    // Test Case 3
    run_test_case({0, -1, -5}); // Expected: even
    
    // Test Case 4 (Empty vector)
    run_test_case({}); // Expected: even

    // Test Case 5
    run_test_case({-1, -2, -3, -4, -5}); // Expected: odd

    return 0;
}