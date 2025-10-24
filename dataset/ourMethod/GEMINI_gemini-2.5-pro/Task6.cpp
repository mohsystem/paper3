#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

/**
 * @brief Finds the unique number in a vector where all other numbers are equal.
 * 
 * It is guaranteed that the vector contains at least 3 numbers.
 * 
 * @param arr A constant reference to a vector of doubles.
 * @return The unique double in the vector.
 */
double findUniq(const std::vector<double>& arr) {
    // The problem guarantees the vector size is at least 3.
    // An assertion is used to enforce this during development.
    assert(arr.size() >= 3);

    // If the first two elements are equal, they represent the common number.
    // We then iterate from the third element to find the one that is different.
    if (arr[0] == arr[1]) {
        double common_number = arr[0];
        for (size_t i = 2; i < arr.size(); ++i) {
            if (arr[i] != common_number) {
                return arr[i];
            }
        }
    } else {
        // If the first two elements are different, one of them is the unique number.
        // The third element will be equal to the common number, which resolves the ambiguity.
        if (arr[0] == arr[2]) {
            // arr[0] and arr[2] are the common number, so arr[1] is unique.
            return arr[1];
        } else {
            // arr[1] and arr[2] must be the common number, so arr[0] is unique.
            return arr[0];
        }
    }

    // This code should be unreachable given the problem constraints.
    // Return NaN (Not a Number) to indicate failure if the constraints are violated.
    return NAN;
}

int main() {
    // Test Case 1
    std::vector<double> test1 = { 1, 1, 1, 2, 1, 1 };
    std::cout << "Test 1: Unique in { 1, 1, 1, 2, 1, 1 } is " << findUniq(test1) << std::endl;

    // Test Case 2
    std::vector<double> test2 = { 0, 0, 0.55, 0, 0 };
    std::cout << "Test 2: Unique in { 0, 0, 0.55, 0, 0 } is " << findUniq(test2) << std::endl;
    
    // Test Case 3
    std::vector<double> test3 = { 3, 1, 1, 1, 1 };
    std::cout << "Test 3: Unique in { 3, 1, 1, 1, 1 } is " << findUniq(test3) << std::endl;
    
    // Test Case 4
    std::vector<double> test4 = { 1, 3, 1, 1, 1 };
    std::cout << "Test 4: Unique in { 1, 3, 1, 1, 1 } is " << findUniq(test4) << std::endl;

    // Test Case 5
    std::vector<double> test5 = { -2, -2, -2, -1 };
    std::cout << "Test 5: Unique in { -2, -2, -2, -1 } is " << findUniq(test5) << std::endl;

    return 0;
}