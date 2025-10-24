#include <iostream>
#include <vector>
#include <utility>

// The function takes a constant reference to a vector of integers and a target integer.
// It returns a std::pair of integers representing the indices.
std::pair<int, int> two_sum(const std::vector<int>& numbers, int target) {
    // The input is guaranteed to be valid, so we don't need to check for size < 2.
    for (size_t i = 0; i < numbers.size(); ++i) {
        // Start the inner loop from i + 1 to avoid using the same element twice
        // and to avoid checking the same pair in reverse order.
        for (size_t j = i + 1; j < numbers.size(); ++j) {
            // If the sum of the two numbers equals the target, return their indices.
            if (numbers[i] + numbers[j] == target) {
                // Cast size_t to int for the return type.
                return {static_cast<int>(i), static_cast<int>(j)};
            }
        }
    }
    // According to the problem description, a solution always exists,
    // so this part of the code should not be reached.
    // We return a pair of -1 to indicate failure, as a fallback.
    return {-1, -1};
}

void run_test_case(const std::vector<int>& numbers, int target) {
    std::pair<int, int> result = two_sum(numbers, target);
    std::cout << "Input: {";
    for (size_t i = 0; i < numbers.size(); ++i) {
        std::cout << numbers[i] << (i == numbers.size() - 1 ? "" : ", ");
    }
    std::cout << "}, Target: " << target << " -> Result: {" << result.first << ", " << result.second << "}" << std::endl;
}

int main() {
    // Test Case 1
    run_test_case({1, 2, 3}, 4);

    // Test Case 2
    run_test_case({1234, 5678, 9012}, 14690);

    // Test Case 3
    run_test_case({2, 2, 3}, 4);

    // Test Case 4
    run_test_case({3, 2, 4}, 6);

    // Test Case 5
    run_test_case({2, 7, 11, 15}, 9);

    return 0;
}