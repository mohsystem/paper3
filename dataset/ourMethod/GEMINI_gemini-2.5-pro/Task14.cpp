#include <iostream>
#include <vector>
#include <numeric>
#include <string>

/**
 * @brief Finds an index N in an array where the sum of integers to the left of N
 * is equal to the sum of integers to the right of N.
 *
 * @param arr A constant reference to a vector of integers.
 * @return The lowest index N that satisfies the condition, or -1 if no such index exists.
 */
int find_even_index(const std::vector<int>& arr) {
    // Use long long to prevent potential integer overflow during summation.
    // std::accumulate with a 0LL initial value ensures the summation uses 64-bit integers.
    long long total_sum = std::accumulate(arr.begin(), arr.end(), 0LL);

    long long left_sum = 0;
    for (size_t i = 0; i < arr.size(); ++i) {
        // The sum of the right side is the total sum minus the left sum and the current element.
        long long right_sum = total_sum - left_sum - arr[i];

        if (left_sum == right_sum) {
            return static_cast<int>(i); // Found the equilibrium index.
        }

        // Add the current element to the left sum for the next iteration.
        left_sum += arr[i];
    }

    // If the loop completes, no such index was found.
    return -1;
}

void run_test_case(const std::string& name, const std::vector<int>& arr, int expected) {
    int result = find_even_index(arr);
    std::cout << "Test Case: " << name << "\n";
    std::cout << "Array: {";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "}\n";
    std::cout << "Expected: " << expected << ", Got: " << result << "\n";
    std::cout << (result == expected ? "PASSED" : "FAILED") << "\n\n";
}

int main() {
    // Test Case 1: Standard case from prompt
    run_test_case("Example 1", {1, 2, 3, 4, 3, 2, 1}, 3);

    // Test Case 2: Another standard case from prompt
    run_test_case("Example 2", {1, 100, 50, -51, 1, 1}, 1);

    // Test Case 3: Equilibrium at the first index
    run_test_case("Index 0", {20, 10, -80, 10, 10, 15, 35}, 0);

    // Test Case 4: Equilibrium at the last index
    run_test_case("Last Index", {10, -80, 10, 10, 15, 35, 20}, 6);

    // Test Case 5: No solution exists
    run_test_case("No Solution", {1, 2, 3, 4, 5, 6}, -1);

    return 0;
}