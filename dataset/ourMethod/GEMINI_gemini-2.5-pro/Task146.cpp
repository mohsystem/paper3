#include <iostream>
#include <vector>
#include <numeric> // For std::accumulate
#include <string>

/**
 * @brief Finds the missing number in a vector containing unique integers from 1 to n.
 *
 * @param nums A constant reference to a vector of unique integers from 1 to n
 *             with one number missing.
 * @return The missing number.
 */
int findMissingNumber(const std::vector<int>& nums) {
    // According to the problem description, the input is a vector of unique
    // integers from 1 to n, with one number missing. This means the vector
    // will have a size of n-1.

    // If the vector is empty, it means n=1, the full sequence is {1},
    // and the missing number is 1.
    if (nums.empty()) {
        return 1;
    }

    // The full sequence should contain n numbers. The vector has n-1.
    // So, n is vector size + 1.
    // Use long long to avoid potential integer overflow for large n.
    long long n = nums.size() + 1;

    // Formula for sum of first n natural numbers: n * (n + 1) / 2
    long long expectedSum = n * (n + 1) / 2;

    // Use std::accumulate to sum elements. Initialize with 0LL (long long zero)
    // to ensure the accumulation happens in a long long, preventing overflow.
    long long actualSum = std::accumulate(nums.begin(), nums.end(), 0LL);

    // The difference is the missing number.
    // The result will fit in an int.
    return static_cast<int>(expectedSum - actualSum);
}

// Helper function to print a vector for test cases
void print_vector(const std::vector<int>& vec) {
    std::cout << "{";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "}";
}

void run_test(const std::string& test_name, const std::vector<int>& vec) {
    std::cout << test_name << ": Array=";
    print_vector(vec);
    std::cout << ", Missing=" << findMissingNumber(vec) << std::endl;
}

int main() {
    // 5 test cases
    run_test("Test 1", {1, 2, 4, 5}); // n=5, missing 3
    run_test("Test 2", {2, 3, 1, 5}); // n=5, missing 4
    run_test("Test 3", {1}); // n=2, missing 2
    run_test("Test 4", {}); // n=1, missing 1
    run_test("Test 5", {1, 2, 3, 4, 5, 6, 7, 9, 10}); // n=10, missing 8

    return 0;
}