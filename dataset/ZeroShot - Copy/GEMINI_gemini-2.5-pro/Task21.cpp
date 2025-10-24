#include <iostream>
#include <vector>
#include <algorithm> // For std::min_element

/**
 * Given a vector of integers, removes the smallest value.
 * This function does not mutate the original vector.
 *
 * @param numbers The input vector of integers.
 * @return A new vector with the first occurrence of the smallest value removed.
 *         Returns an empty vector if the input is empty.
 */
std::vector<int> removeSmallest(const std::vector<int>& numbers) {
    if (numbers.empty()) {
        return {}; // Return an empty vector
    }

    // Create a copy to avoid mutating the original vector
    std::vector<int> result = numbers;
    
    // Find an iterator to the first occurrence of the smallest element
    auto min_it = std::min_element(result.begin(), result.end());
    
    // Erase the element pointed to by the iterator
    if (min_it != result.end()) {
        result.erase(min_it);
    }
    
    return result;
}

// Helper function to print a vector
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    std::vector<std::vector<int>> testCases = {
        {1, 2, 3, 4, 5},
        {5, 3, 2, 1, 4},
        {2, 2, 1, 2, 1},
        {1},
        {}
    };

    for (const auto& testCase : testCases) {
        std::vector<int> result = removeSmallest(testCase);
        printVector(result);
    }

    return 0;
}