#include <iostream>
#include <vector>
#include <algorithm> // For std::min_element
#include <string>

/**
 * Given a vector of integers, remove the smallest value.
 * Do not mutate the original vector.
 * If there are multiple elements with the same value, remove the one with the lowest index.
 * If you get an empty vector, return an empty vector.
 *
 * @param numbers The input const reference to a vector of integers.
 * @return A new vector with the smallest element removed.
 */
std::vector<int> removeSmallest(const std::vector<int>& numbers) {
    if (numbers.empty()) {
        return {};
    }

    // Create a copy to not mutate the original vector.
    std::vector<int> result = numbers;
    
    // Find an iterator to the first occurrence of the minimum element.
    auto minIt = std::min_element(result.begin(), result.end());
    
    // Erase the element at that iterator's position.
    if (minIt != result.end()) {
        result.erase(minIt);
    }
    
    return result;
}

// Helper function to print a vector
void printVector(const std::string& label, const std::vector<int>& vec) {
    std::cout << label;
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    std::vector<std::vector<int>> test_cases = {
        {1, 2, 3, 4, 5},
        {5, 3, 2, 1, 4},
        {2, 2, 1, 2, 1},
        {10},
        {}
    };

    for (const auto& test_case : test_cases) {
        printVector("Input:  ", test_case);
        std::vector<int> result = removeSmallest(test_case);
        printVector("Output: ", result);
        std::cout << "---" << std::endl;
    }

    return 0;
}