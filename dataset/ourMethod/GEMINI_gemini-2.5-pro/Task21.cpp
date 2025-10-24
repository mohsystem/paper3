#include <iostream>
#include <vector>
#include <limits>
#include <algorithm> // For std::copy
#include <iterator>  // For std::ostream_iterator

// Function to remove the smallest element from a vector.
// Does not mutate the original vector.
std::vector<int> removeSmallest(const std::vector<int>& numbers) {
    if (numbers.empty()) {
        return {};
    }

    int min_val = std::numeric_limits<int>::max();
    size_t min_index = 0;

    for (size_t i = 0; i < numbers.size(); ++i) {
        if (numbers[i] < min_val) {
            min_val = numbers[i];
            min_index = i;
        }
    }

    std::vector<int> result;
    if (numbers.size() > 1) {
        result.reserve(numbers.size() - 1);
    }
    
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (i != min_index) {
            result.push_back(numbers[i]);
        }
    }

    return result;
}

// Helper function to print a vector
void printVector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix << "[";
    if (!vec.empty()) {
        std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<int>(std::cout, ", "));
        std::cout << vec.back();
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1
    std::vector<int> test1 = {1, 2, 3, 4, 5};
    printVector("Input:  ", test1);
    std::vector<int> result1 = removeSmallest(test1);
    printVector("Output: ", result1);
    std::cout << "--------------------" << std::endl;

    // Test Case 2
    std::vector<int> test2 = {5, 3, 2, 1, 4};
    printVector("Input:  ", test2);
    std::vector<int> result2 = removeSmallest(test2);
    printVector("Output: ", result2);
    std::cout << "--------------------" << std::endl;

    // Test Case 3
    std::vector<int> test3 = {2, 2, 1, 2, 1};
    printVector("Input:  ", test3);
    std::vector<int> result3 = removeSmallest(test3);
    printVector("Output: ", result3);
    std::cout << "--------------------" << std::endl;
    
    // Test Case 4: Single element
    std::vector<int> test4 = {5};
    printVector("Input:  ", test4);
    std::vector<int> result4 = removeSmallest(test4);
    printVector("Output: ", result4);
    std::cout << "--------------------" << std::endl;

    // Test Case 5: Empty list
    std::vector<int> test5 = {};
    printVector("Input:  ", test5);
    std::vector<int> result5 = removeSmallest(test5);
    printVector("Output: ", result5);
    std::cout << "--------------------" << std::endl;

    return 0;
}