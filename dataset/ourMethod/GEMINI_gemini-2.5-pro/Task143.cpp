#include <iostream>
#include <vector>
#include <algorithm> // for std::sort
#include <iterator> // for std::ostream_iterator

/**
 * @brief Prints the elements of a vector.
 * @param arr The vector to be printed.
 */
void printVector(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "]";
}

/**
 * @brief Sorts a vector of integers in ascending order.
 * The sorting is done in-place.
 *
 * @param arr A reference to the vector of integers to be sorted.
 */
void sortArray(std::vector<int>& arr) {
    // std::sort is a highly optimized and safe sorting algorithm.
    // It correctly handles the vector's boundaries (begin() to end()),
    // adhering to Rule #4. It also safely handles empty vectors.
    std::sort(arr.begin(), arr.end());
}

/**
 * @brief Main function for testing the sortArray function.
 * @return int Exit code.
 */
int main() {
    // Test cases
    std::vector<std::vector<int>> testCases = {
        {},                                 // Test Case 1: Empty vector
        {5, -2, 9, 1, 5, 6},                // Test Case 2: Mixed positive and negative numbers
        {1},                                // Test Case 3: Single element vector
        {9, 8, 7, 6, 5, 4, 3, 2, 1},        // Test Case 4: Reverse sorted vector
        {1, 2, 3, 4, 5, 6, 7, 8, 9}         // Test Case 5: Already sorted vector
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Original array: ";
        printVector(testCases[i]);
        std::cout << std::endl;
        
        sortArray(testCases[i]);
        
        std::cout << "Sorted array:   ";
        printVector(testCases[i]);
        std::cout << std::endl << std::endl;
    }

    return 0;
}