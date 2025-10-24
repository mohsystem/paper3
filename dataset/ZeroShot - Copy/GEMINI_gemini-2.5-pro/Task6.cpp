#include <iostream>
#include <vector>
#include <stdexcept>

/**
 * @brief Finds the unique number in a vector where all other numbers are equal.
 *
 * It's guaranteed that the vector contains at least 3 numbers.
 *
 * @param arr The input vector of doubles.
 * @return The unique number in the vector.
 */
double findUniq(const std::vector<double>& arr) {
    // Compare the first two elements. If they are different, the unique
    // number is one of them. The third element decides which one.
    if (arr[0] != arr[1]) {
        return (arr[0] == arr[2]) ? arr[1] : arr[0];
    } else {
        // If the first two are the same, this is the common number.
        // Iterate through the rest to find the different one.
        double commonNumber = arr[0];
        for (size_t i = 2; i < arr.size(); ++i) {
            if (arr[i] != commonNumber) {
                return arr[i];
            }
        }
    }
    // This part should be unreachable given the problem constraints.
    // Throwing an exception is a safe way to handle unexpected scenarios.
    throw std::invalid_argument("No unique number found in the vector.");
}

int main() {
    // Test Case 1
    std::vector<double> arr1 = {1, 1, 1, 2, 1, 1};
    std::cout << "Test 1 Result: " << findUniq(arr1) << std::endl;

    // Test Case 2
    std::vector<double> arr2 = {0, 0, 0.55, 0, 0};
    std::cout << "Test 2 Result: " << findUniq(arr2) << std::endl;

    // Test Case 3: Unique at the beginning
    std::vector<double> arr3 = {3, 1, 1, 1, 1, 1};
    std::cout << "Test 3 Result: " << findUniq(arr3) << std::endl;

    // Test Case 4: Unique at the end
    std::vector<double> arr4 = {4, 4, 4, 4, 3.5};
    std::cout << "Test 4 Result: " << findUniq(arr4) << std::endl;
    
    // Test Case 5: Negative numbers
    std::vector<double> arr5 = {-1, -1, -1, -2, -1};
    std::cout << "Test 5 Result: " << findUniq(arr5) << std::endl;

    return 0;
}