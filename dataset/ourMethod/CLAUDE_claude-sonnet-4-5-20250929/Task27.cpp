
#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

// Function to find the outlier in an array of integers
// Security: All inputs validated, bounds checked, no dynamic memory issues
int findOutlier(const std::vector<int>& integers) {
    // Input validation: check minimum array size
    if (integers.size() < 3) {
        throw std::invalid_argument("Array must have at least 3 elements");
    }
    
    // Security: Check for maximum reasonable size to prevent excessive memory usage
    if (integers.size() > 1000000) {
        throw std::invalid_argument("Array size exceeds maximum allowed limit");
    }
    
    // Count even and odd numbers in first 3 elements to determine majority type
    // This prevents unnecessary full array traversal and provides early determination
    int evenCount = 0;
    int oddCount = 0;
    
    // Check first 3 elements with bounds guarantee (already validated size >= 3)
    for (size_t i = 0; i < 3; ++i) {
        if (integers[i] % 2 == 0) {
            evenCount++;
        } else {
            oddCount++;
        }
    }
    
    // Determine if we're looking for an even or odd outlier
    // Majority in first 3 elements determines the array type
    bool lookingForEven = (oddCount > evenCount);
    
    // Find and return the outlier
    // Security: using size_t for index to prevent signed integer issues
    for (size_t i = 0; i < integers.size(); ++i) {
        bool isEven = (integers[i] % 2 == 0);
        if (lookingForEven && isEven) {
            return integers[i];
        }
        if (!lookingForEven && !isEven) {
            return integers[i];
        }
    }
    
    // This should never be reached given problem constraints, but included for safety
    throw std::runtime_error("No outlier found in array");
}

int main() {
    // Test case 1: Outlier is odd
    try {
        std::vector<int> test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        std::cout << "Test 1: " << findOutlier(test1) << " (expected: 11)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: Outlier is even
    try {
        std::vector<int> test2 = {160, 3, 1719, 19, 11, 13, -21};
        std::cout << "Test 2: " << findOutlier(test2) << " (expected: 160)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: Outlier at beginning
    try {
        std::vector<int> test3 = {1, 2, 4, 6, 8, 10};
        std::cout << "Test 3: " << findOutlier(test3) << " (expected: 1)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: Outlier at end
    try {
        std::vector<int> test4 = {2, 4, 6, 8, 10, 3};
        std::cout << "Test 4: " << findOutlier(test4) << " (expected: 3)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: Negative numbers
    try {
        std::vector<int> test5 = {-1, -3, -5, -7, 2};
        std::cout << "Test 5: " << findOutlier(test5) << " (expected: 2)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
