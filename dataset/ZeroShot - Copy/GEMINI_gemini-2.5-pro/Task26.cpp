#include <iostream>
#include <vector>
#include <numeric>

/**
 * @brief Finds the integer that appears an odd number of times in a vector.
 *
 * It uses the XOR bitwise operator. The property of XOR is that a^a = 0 and a^0 = a.
 * When we XOR all numbers, pairs of identical numbers cancel out, leaving the unique one.
 *
 * @param arr The input vector of integers. It is assumed that there will always be
 *            only one integer that appears an odd number of times.
 * @return The integer that appears an odd number of times.
 */
int findOdd(const std::vector<int>& arr) {
    int result = 0;
    for (int num : arr) {
        result ^= num;
    }
    return result;
}

void printVector(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

int main() {
    // Test Case 1
    std::vector<int> test1 = {7};
    std::cout << "Input: "; printVector(test1); std::cout << ", Output: " << findOdd(test1) << std::endl;

    // Test Case 2
    std::vector<int> test2 = {0};
    std::cout << "Input: "; printVector(test2); std::cout << ", Output: " << findOdd(test2) << std::endl;

    // Test Case 3
    std::vector<int> test3 = {1, 1, 2};
    std::cout << "Input: "; printVector(test3); std::cout << ", Output: " << findOdd(test3) << std::endl;

    // Test Case 4
    std::vector<int> test4 = {0, 1, 0, 1, 0};
    std::cout << "Input: "; printVector(test4); std::cout << ", Output: " << findOdd(test4) << std::endl;

    // Test Case 5
    std::vector<int> test5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    std::cout << "Input: "; printVector(test5); std::cout << ", Output: " << findOdd(test5) << std::endl;

    return 0;
}