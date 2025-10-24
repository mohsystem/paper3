
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>

// Function to find kth largest element in array
// Returns the kth largest element (1-indexed, where 1 is the largest)
// Throws exception if inputs are invalid
int findKthLargest(const std::vector<int>& nums, int k) {
    // Input validation: check if array is empty
    if (nums.empty()) {
        throw std::invalid_argument("Array cannot be empty");
    }
    
    // Input validation: check if k is in valid range [1, array size]
    if (k < 1 || k > static_cast<int>(nums.size())) {
        throw std::invalid_argument("k must be between 1 and array size");
    }
    
    // Create a copy to avoid modifying original array
    // This prevents side effects and is safer for multi-threaded contexts
    std::vector<int> sortedNums = nums;
    
    // Sort in descending order to find kth largest
    // Using standard library sort which is safe and well-tested
    std::sort(sortedNums.begin(), sortedNums.end(), std::greater<int>());
    
    // Return kth largest (k-1 because of 0-based indexing)
    // Bounds already validated above, so this access is safe
    return sortedNums[k - 1];
}

int main() {
    try {
        // Test case 1: Normal case with distinct elements
        std::vector<int> test1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        std::cout << "Test 1: Array [3,2,1,5,6,4], k=2" << std::endl;
        std::cout << "Result: " << findKthLargest(test1, k1) << std::endl;
        std::cout << "Expected: 5" << std::endl << std::endl;
        
        // Test case 2: Array with duplicates
        std::vector<int> test2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        std::cout << "Test 2: Array [3,2,3,1,2,4,5,5,6], k=4" << std::endl;
        std::cout << "Result: " << findKthLargest(test2, k2) << std::endl;
        std::cout << "Expected: 4" << std::endl << std::endl;
        
        // Test case 3: Single element array
        std::vector<int> test3 = {1};
        int k3 = 1;
        std::cout << "Test 3: Array [1], k=1" << std::endl;
        std::cout << "Result: " << findKthLargest(test3, k3) << std::endl;
        std::cout << "Expected: 1" << std::endl << std::endl;
        
        // Test case 4: Array with negative numbers
        std::vector<int> test4 = {-1, -2, -3, -4, -5};
        int k4 = 3;
        std::cout << "Test 4: Array [-1,-2,-3,-4,-5], k=3" << std::endl;
        std::cout << "Result: " << findKthLargest(test4, k4) << std::endl;
        std::cout << "Expected: -3" << std::endl << std::endl;
        
        // Test case 5: Large values including INT_MAX and INT_MIN
        std::vector<int> test5 = {std::numeric_limits<int>::max(), 0, std::numeric_limits<int>::min(), 100, -100};
        int k5 = 2;
        std::cout << "Test 5: Array [INT_MAX,0,INT_MIN,100,-100], k=2" << std::endl;
        std::cout << "Result: " << findKthLargest(test5, k5) << std::endl;
        std::cout << "Expected: 100" << std::endl << std::endl;
        
        // Test case 6: Invalid k (error handling test)
        std::cout << "Test 6: Invalid k (should throw exception)" << std::endl;
        try {
            std::vector<int> test6 = {1, 2, 3};
            findKthLargest(test6, 5);
            std::cout << "Error: Exception not thrown" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Correctly caught exception: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        // Global exception handler for any unexpected errors
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
