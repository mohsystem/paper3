#include <iostream>
#include <vector>

/**
 * Finds two indices of numbers in a vector that add up to a target value.
 *
 * @param numbers The input vector of integers.
 * @param target The target sum.
 * @return A vector containing the two indices. Returns an empty vector if no solution is found.
 */
std::vector<int> twoSum(const std::vector<int>& numbers, int target) {
    for (size_t i = 0; i < numbers.size(); ++i) {
        for (size_t j = i + 1; j < numbers.size(); ++j) {
            if (numbers[i] + numbers[j] == target) {
                return {(int)i, (int)j};
            }
        }
    }
    // As per problem description, a solution always exists.
    // This is to satisfy the compiler for a return path.
    return {};
}

int main() {
    // Test Case 1
    std::vector<int> result1 = twoSum({1, 2, 3}, 4);
    std::cout << "Test Case 1: {" << result1[0] << ", " << result1[1] << "}\n";

    // Test Case 2
    std::vector<int> result2 = twoSum({1234, 5678, 9012}, 14690);
    std::cout << "Test Case 2: {" << result2[0] << ", " << result2[1] << "}\n";

    // Test Case 3
    std::vector<int> result3 = twoSum({2, 2, 3}, 4);
    std::cout << "Test Case 3: {" << result3[0] << ", " << result3[1] << "}\n";

    // Test Case 4
    std::vector<int> result4 = twoSum({3, 2, 4}, 6);
    std::cout << "Test Case 4: {" << result4[0] << ", " << result4[1] << "}\n";

    // Test Case 5
    std::vector<int> result5 = twoSum({2, 7, 11, 15}, 9);
    std::cout << "Test Case 5: {" << result5[0] << ", " << result5[1] << "}\n";

    return 0;
}