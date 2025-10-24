#include <iostream>
#include <vector>
#include <algorithm> // for std::sort
#include <iterator> // for std::ostream_iterator

/**
 * Helper function to print a vector of integers.
 * @param vec The vector to print.
 */
void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    if (!vec.empty()) {
        std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<int>(std::cout, ", "));
        std::cout << vec.back();
    }
    std::cout << "]";
}

/**
 * Sorts a vector of integers in ascending order.
 * This function sorts the vector in-place.
 *
 * @param arr The vector of integers to be sorted.
 */
void sortArray(std::vector<int>& arr) {
    // Secure: Using std::sort, which is typically an Introsort implementation.
    // It's a hybrid algorithm that avoids the worst-case O(n^2) performance
    // of a naive quicksort, preventing potential DoS attacks.
    std::sort(arr.begin(), arr.end());
}

int main() {
    // Test Case 1: Regular unsorted vector
    std::cout << "--- Test Case 1: Regular unsorted vector ---" << std::endl;
    std::vector<int> test1 = {5, 2, 8, 1, 9, 4};
    std::cout << "Before: ";
    printVector(test1);
    std::cout << std::endl;
    sortArray(test1);
    std::cout << "After:  ";
    printVector(test1);
    std::cout << std::endl << std::endl;

    // Test Case 2: vector with negative numbers and duplicates
    std::cout << "--- Test Case 2: vector with negative numbers and duplicates ---" << std::endl;
    std::vector<int> test2 = {-5, 2, -8, 2, 9, 4, -5};
    std::cout << "Before: ";
    printVector(test2);
    std::cout << std::endl;
    sortArray(test2);
    std::cout << "After:  ";
    printVector(test2);
    std::cout << std::endl << std::endl;

    // Test Case 3: Already sorted vector
    std::cout << "--- Test Case 3: Already sorted vector ---" << std::endl;
    std::vector<int> test3 = {1, 2, 3, 4, 5, 6};
    std::cout << "Before: ";
    printVector(test3);
    std::cout << std::endl;
    sortArray(test3);
    std::cout << "After:  ";
    printVector(test3);
    std::cout << std::endl << std::endl;

    // Test Case 4: Reverse sorted vector
    std::cout << "--- Test Case 4: Reverse sorted vector ---" << std::endl;
    std::vector<int> test4 = {10, 8, 6, 4, 2, 0};
    std::cout << "Before: ";
    printVector(test4);
    std::cout << std::endl;
    sortArray(test4);
    std::cout << "After:  ";
    printVector(test4);
    std::cout << std::endl << std::endl;

    // Test Case 5: Empty vector
    std::cout << "--- Test Case 5: Edge case (empty vector) ---" << std::endl;
    std::vector<int> test5_empty;
    std::cout << "Before (empty): ";
    printVector(test5_empty);
    std::cout << std::endl;
    sortArray(test5_empty);
    std::cout << "After (empty):  ";
    printVector(test5_empty);
    std::cout << std::endl << std::endl;

    return 0;
}