#include <iostream>
#include <vector>
#include <numeric>

// Function to find the integer that appears an odd number of times.
// It uses the XOR bitwise operator property where a ^ a = 0.
// All numbers appearing an even number of times will cancel out,
// leaving the one that appears an odd number of times.
int findOdd(const std::vector<int>& arr) {
    int result = 0;
    for (int num : arr) {
        result ^= num;
    }
    return result;
}

// Helper function to run and print a test case
void run_test_case(const std::vector<int>& test_data) {
    std::cout << "Input: { ";
    for (size_t i = 0; i < test_data.size(); ++i) {
        std::cout << test_data[i] << (i == test_data.size() - 1 ? "" : ", ");
    }
    int result = findOdd(test_data);
    std::cout << " } -> Result: " << result << std::endl;
}

int main() {
    // Test Case 1
    run_test_case({7});

    // Test Case 2
    run_test_case({0});

    // Test Case 3
    run_test_case({1, 1, 2});

    // Test Case 4
    run_test_case({0, 1, 0, 1, 0});

    // Test Case 5
    run_test_case({1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1});

    return 0;
}