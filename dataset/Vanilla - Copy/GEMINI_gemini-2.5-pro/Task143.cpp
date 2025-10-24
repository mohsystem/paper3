#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

/**
 * Sorts a vector of integers in ascending order.
 * This function takes the vector by value to return a new sorted vector,
 * leaving the original unchanged.
 *
 * @param arr The vector of integers to sort.
 * @return A new vector containing the sorted integers.
 */
std::vector<int> sortArray(std::vector<int> arr) {
    std::sort(arr.begin(), arr.end());
    return arr;
}

/**
 * Helper function to print a vector of integers.
 * @param label A description for the output.
 * @param vec The vector to print.
 */
void printVector(const std::string& label, const std::vector<int>& vec) {
    std::cout << label;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : " ");
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1: Standard unsorted array
    std::vector<int> arr1 = {5, 2, 8, 1, 9};
    std::cout << "Test Case 1" << std::endl;
    printVector("Original: ", arr1);
    std::vector<int> sorted_arr1 = sortArray(arr1);
    printVector("Sorted:   ", sorted_arr1);
    std::cout << std::endl;

    // Test Case 2: Reverse-sorted array
    std::vector<int> arr2 = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::cout << "Test Case 2" << std::endl;
    printVector("Original: ", arr2);
    std::vector<int> sorted_arr2 = sortArray(arr2);
    printVector("Sorted:   ", sorted_arr2);
    std::cout << std::endl;

    // Test Case 3: Already sorted array
    std::vector<int> arr3 = {1, 2, 3, 4, 5};
    std::cout << "Test Case 3" << std::endl;
    printVector("Original: ", arr3);
    std::vector<int> sorted_arr3 = sortArray(arr3);
    printVector("Sorted:   ", sorted_arr3);
    std::cout << std::endl;

    // Test Case 4: Array with duplicate elements
    std::vector<int> arr4 = {4, 2, 7, 2, 5, 4};
    std::cout << "Test Case 4" << std::endl;
    printVector("Original: ", arr4);
    std::vector<int> sorted_arr4 = sortArray(arr4);
    printVector("Sorted:   ", sorted_arr4);
    std::cout << std::endl;

    // Test Case 5: Array with negative numbers and zero
    std::vector<int> arr5 = {-5, 0, 10, -2, 3};
    std::cout << "Test Case 5" << std::endl;
    printVector("Original: ", arr5);
    std::vector<int> sorted_arr5 = sortArray(arr5);
    printVector("Sorted:   ", sorted_arr5);
    std::cout << std::endl;

    return 0;
}