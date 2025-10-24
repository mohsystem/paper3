#include <iostream>
#include <vector>
#include <algorithm>

/**
 * @brief Sorts a vector of integers in ascending order.
 * 
 * @param arr The vector of integers to be sorted (passed by reference).
 */
void sortArray(std::vector<int>& arr) {
    // std::sort handles empty ranges correctly.
    std::sort(arr.begin(), arr.end());
}

/**
 * @brief Helper function to print a vector of integers.
 * 
 * @param arr The vector to print.
 */
void printVector(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i];
        if (i < arr.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    // Test Case 1: General case
    std::vector<int> arr1 = {5, 2, 8, 1, 9, 4};
    std::cout << "Test Case 1:\n";
    std::cout << "Original: ";
    printVector(arr1);
    std::cout << "\n";
    sortArray(arr1);
    std::cout << "Sorted:   ";
    printVector(arr1);
    std::cout << "\n\n";

    // Test Case 2: Already sorted
    std::vector<int> arr2 = {1, 2, 3, 4, 5, 6};
    std::cout << "Test Case 2:\n";
    std::cout << "Original: ";
    printVector(arr2);
    std::cout << "\n";
    sortArray(arr2);
    std::cout << "Sorted:   ";
    printVector(arr2);
    std::cout << "\n\n";

    // Test Case 3: Reverse sorted
    std::vector<int> arr3 = {10, 8, 6, 4, 2, 0};
    std::cout << "Test Case 3:\n";
    std::cout << "Original: ";
    printVector(arr3);
    std::cout << "\n";
    sortArray(arr3);
    std::cout << "Sorted:   ";
    printVector(arr3);
    std::cout << "\n\n";

    // Test Case 4: With duplicates and negative numbers
    std::vector<int> arr4 = {4, -1, 5, 2, 4, -1, 0};
    std::cout << "Test Case 4:\n";
    std::cout << "Original: ";
    printVector(arr4);
    std::cout << "\n";
    sortArray(arr4);
    std::cout << "Sorted:   ";
    printVector(arr4);
    std::cout << "\n\n";

    // Test Case 5: Empty vector
    std::vector<int> arr5 = {};
    std::cout << "Test Case 5:\n";
    std::cout << "Original: ";
    printVector(arr5);
    std::cout << "\n";
    sortArray(arr5);
    std::cout << "Sorted:   ";
    printVector(arr5);
    std::cout << "\n\n";

    return 0;
}