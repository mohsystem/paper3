#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>

class Task29 {
public:
    /**
     * @brief Finds two different items in the vector that sum up to the target value.
     * This implementation uses an unordered_map for O(n) time complexity.
     * @param numbers A constant reference to a vector of integers.
     * @param target The target sum.
     * @return A vector containing the indices of the two numbers.
     */
    static std::vector<int> twoSum(const std::vector<int>& numbers, int target) {
        std::unordered_map<int, int> num_map;
        for (int i = 0; i < numbers.size(); ++i) {
            int complement = target - numbers[i];
            if (num_map.count(complement)) {
                return {num_map[complement], i};
            }
            num_map[numbers[i]] = i;
        }
        // According to the problem description, a solution always exists.
        // Throwing an exception is appropriate for an unreachable state.
        throw std::invalid_argument("No two sum solution found");
    }
};

int main() {
    // Test cases
    std::vector<std::pair<std::vector<int>, int>> test_cases = {
        {{1, 2, 3}, 4},
        {{1234, 5678, 9012}, 14690},
        {{2, 2, 3}, 4},
        {{3, 2, 4}, 6},
        {{2, 7, 11, 15}, 9}
    };

    for (int i = 0; i < test_cases.size(); ++i) {
        std::vector<int> result = Task29::twoSum(test_cases[i].first, test_cases[i].second);
        
        // Sort for consistent output format
        std::sort(result.begin(), result.end());

        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Input: numbers = [";
        for (size_t j = 0; j < test_cases[i].first.size(); ++j) {
            std::cout << test_cases[i].first[j] << (j == test_cases[i].first.size() - 1 ? "" : ", ");
        }
        std::cout << "], target = " << test_cases[i].second << std::endl;
        
        std::cout << "Output: {" << result[0] << ", " << result[1] << "}" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}