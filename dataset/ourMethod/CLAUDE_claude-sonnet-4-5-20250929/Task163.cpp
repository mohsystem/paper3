
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

// Function to find the longest consecutive run in an array
// Security: Validates input size, uses safe iterators, checks for integer overflow
int longestRun(const std::vector<int>& arr) {
    // Input validation: empty array check
    if (arr.empty()) {
        throw std::invalid_argument("Array cannot be empty");
    }
    
    // Input validation: size check to prevent potential integer overflow
    if (arr.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Array size exceeds maximum safe value");
    }
    
    // Base case: single element returns 1
    if (arr.size() == 1) {
        return 1;
    }
    
    int maxRun = 1;  // Initialize to 1 as per problem requirements
    int currentRun = 1;
    
    // Iterate through array safely using size_t to avoid overflow
    for (size_t i = 1; i < arr.size(); ++i) {
        // Security: Check bounds before accessing array elements
        if (i >= arr.size()) {
            break;
        }
        
        // Calculate difference safely to avoid overflow
        // Check if consecutive (diff is +1 or -1)
        long long diff = static_cast<long long>(arr[i]) - static_cast<long long>(arr[i-1]);
        
        if (diff == 1 || diff == -1) {
            // Security: Check for integer overflow before incrementing
            if (currentRun == std::numeric_limits<int>::max()) {
                throw std::overflow_error("Run length exceeds maximum integer value");
            }
            currentRun++;
        } else {
            // Update maxRun and reset currentRun
            maxRun = std::max(maxRun, currentRun);
            currentRun = 1;
        }
    }
    
    // Final comparison to catch runs that end at array boundary
    maxRun = std::max(maxRun, currentRun);
    
    return maxRun;
}

int main() {
    // Test case 1: Mixed consecutive runs
    std::vector<int> test1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test 1: " << longestRun(test1) << " (expected: 5)" << std::endl;
    
    // Test case 2: Consecutive run at start
    std::vector<int> test2 = {1, 2, 3, 10, 11, 15};
    std::cout << "Test 2: " << longestRun(test2) << " (expected: 3)" << std::endl;
    
    // Test case 3: Decreasing consecutive runs
    std::vector<int> test3 = {5, 4, 2, 1};
    std::cout << "Test 3: " << longestRun(test3) << " (expected: 2)" << std::endl;
    
    // Test case 4: No consecutive runs
    std::vector<int> test4 = {3, 5, 7, 10, 15};
    std::cout << "Test 4: " << longestRun(test4) << " (expected: 1)" << std::endl;
    
    // Test case 5: Single element
    std::vector<int> test5 = {42};
    std::cout << "Test 5: " << longestRun(test5) << " (expected: 1)" << std::endl;
    
    return 0;
}
