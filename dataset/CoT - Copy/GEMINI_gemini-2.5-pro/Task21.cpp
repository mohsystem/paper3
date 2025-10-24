#include <iostream>
#include <vector>
#include <algorithm> // For std::min_element
#include <limits>    // For std::numeric_limits

/**
 * Given a vector of integers, remove the smallest value.
 * Do not mutate the original vector.
 * If there are multiple elements with the same value, remove the one with the lowest index.
 * If you get an empty vector, return an empty vector.
 *
 * @param numbers The input constant reference to a vector of integers.
 * @return A new vector with the smallest value removed.
 */
std::vector<int> removeSmallest(const std::vector<int>& numbers) {
    if (numbers.empty()) {
        return {};
    }

    int min_index = 0;
    int min_value = std::numeric_limits<int>::max();

    for (size_t i = 0; i < numbers.size(); ++i) {
        if (numbers[i] < min_value) {
            min_value = numbers[i];
            min_index = i;
        }
    }

    std::vector<int> result;
    result.reserve(numbers.size() - 1); // Pre-allocate memory for efficiency

    for (size_t i = 0; i < numbers.size(); ++i) {
        if (i != static_cast<size_t>(min_index)) {
            result.push_back(numbers[i]);
        }
    }
    
    return result;
}

// Helper function to print a vector
void printVector(const std::string& label, const std::vector<int>& vec) {
    std::cout << label << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Cases
    std::vector<std::vector<int>> testCases = {
        {1, 2, 3, 4, 5},
        {5, 3, 2, 1, 4},
        {2, 2, 1, 2, 1},
        {},
        {3, 3, 3, 3, 3}
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        const auto& original = testCases[i];
        auto result = removeSmallest(original);
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        printVector("Original: ", original);
        printVector("Result:   ", result);
        std::cout << std::endl;
    }

    return 0;
}