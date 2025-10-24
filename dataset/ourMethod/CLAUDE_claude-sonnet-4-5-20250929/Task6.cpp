
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <stdexcept>

// Find the unique number in an array where all other numbers are equal
// Input validation: array must have at least 3 elements
// Security: validates array size to prevent out-of-bounds access
double findUniq(const std::vector<double>& arr) {
    // Input validation: ensure array has at least 3 elements as guaranteed
    if (arr.size() < 3) {
        throw std::invalid_argument("Array must contain at least 3 elements");
    }
    
    // Compare first three elements to determine which value is unique
    // This is safe as we validated size >= 3
    double first = arr[0];
    double second = arr[1];
    double third = arr[2];
    
    // Use epsilon for floating point comparison to handle precision issues
    const double epsilon = std::numeric_limits<double>::epsilon() * 100;
    
    // Determine which value appears more than once (the common value)
    double common;
    double unique;
    
    if (std::fabs(first - second) < epsilon) {
        // first == second, so third might be unique
        common = first;
        unique = third;
    } else if (std::fabs(first - third) < epsilon) {
        // first == third, so second is unique
        common = first;
        unique = second;
    } else {
        // second == third, so first is unique
        common = second;
        unique = first;
    }
    
    // Verify by counting occurrences to ensure correctness
    // This prevents issues if our initial assumption was wrong
    size_t commonCount = 0;
    size_t uniqueCount = 0;
    
    for (size_t i = 0; i < arr.size(); ++i) {
        if (std::fabs(arr[i] - common) < epsilon) {
            ++commonCount;
        } else if (std::fabs(arr[i] - unique) < epsilon) {
            ++uniqueCount;
        }
    }
    
    // Return the value that appears only once
    if (uniqueCount == 1) {
        return unique;
    } else if (commonCount == 1) {
        return common;
    }
    
    // Should not reach here with valid input
    throw std::runtime_error("Could not find unique element");
}

int main() {
    // Test case 1: unique in middle
    std::vector<double> test1 = {1, 1, 1, 2, 1, 1};
    std::cout << "Test 1: " << findUniq(test1) << " (expected: 2)" << std::endl;
    
    // Test case 2: unique decimal
    std::vector<double> test2 = {0, 0, 0.55, 0, 0};
    std::cout << "Test 2: " << findUniq(test2) << " (expected: 0.55)" << std::endl;
    
    // Test case 3: unique at start
    std::vector<double> test3 = {5, 3, 3, 3, 3};
    std::cout << "Test 3: " << findUniq(test3) << " (expected: 5)" << std::endl;
    
    // Test case 4: unique at end
    std::vector<double> test4 = {7, 7, 7, 7, 9};
    std::cout << "Test 4: " << findUniq(test4) << " (expected: 9)" << std::endl;
    
    // Test case 5: negative numbers
    std::vector<double> test5 = {-1, -1, -1, -2, -1};
    std::cout << "Test 5: " << findUniq(test5) << " (expected: -2)" << std::endl;
    
    return 0;
}
