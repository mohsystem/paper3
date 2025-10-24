#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>

/**
 * @brief Finds two indices of numbers in a vector that add up to a target value.
 * 
 * @param numbers The input vector of integers.
 * @param target The target sum.
 * @return A vector of two integers representing the indices of the two numbers.
 */
std::vector<int> two_sum(const std::vector<int>& numbers, int target) {
    std::unordered_map<int, int> num_map;
    for (int i = 0; i < numbers.size(); ++i) {
        int complement = target - numbers[i];
        if (num_map.count(complement)) {
            return {num_map.at(complement), i};
        }
        num_map[numbers[i]] = i;
    }
    // According to the problem description, a solution always exists.
    throw std::invalid_argument("No two sum solution");
}

int main() {
    // Helper lambda to print a vector
    auto print_vector = [](const std::vector<int>& vec) {
        if (vec.size() == 2) {
            std::cout << "[" << vec[0] << ", " << vec[1] << "]" << std::endl;
        }
    };

    // Test Case 1
    std::vector<int> result1 = two_sum({1, 2, 3}, 4);
    std::cout << "Test 1: ";
    print_vector(result1); // Expected: [0, 2]

    // Test Case 2
    std::vector<int> result2 = two_sum({1234, 5678, 9012}, 14690);
    std::cout << "Test 2: ";
    print_vector(result2); // Expected: [1, 2]

    // Test Case 3
    std::vector<int> result3 = two_sum({2, 2, 3}, 4);
    std::cout << "Test 3: ";
    print_vector(result3); // Expected: [0, 1]

    // Test Case 4
    std::vector<int> result4 = two_sum({3, 2, 4}, 6);
    std::cout << "Test 4: ";
    print_vector(result4); // Expected: [1, 2]
    
    // Test Case 5
    std::vector<int> result5 = two_sum({2, 7, 11, 15}, 9);
    std::cout << "Test 5: ";
    print_vector(result5); // Expected: [0, 1]

    return 0;
}