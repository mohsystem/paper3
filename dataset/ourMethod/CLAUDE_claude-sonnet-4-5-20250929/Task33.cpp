
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>

// Function to calculate total checkout time
// Security: All inputs validated, bounds checked, no dynamic allocation vulnerabilities
int queueTime(const std::vector<int>& customers, int n) {
    // Input validation: check for negative or zero tills
    if (n <= 0) {
        return 0;
    }
    
    // Input validation: check for empty customer queue
    if (customers.empty()) {
        return 0;
    }
    
    // Input validation: ensure n doesn't exceed reasonable bounds to prevent excessive memory\n    const int MAX_TILLS = 100000;\n    if (n > MAX_TILLS) {\n        n = MAX_TILLS;\n    }\n    \n    // Input validation: validate customer times are non-negative\n    for (size_t i = 0; i < customers.size(); i++) {\n        if (customers[i] < 0) {\n            return 0; // Invalid input\n        }\n    }\n    \n    // Limit n to actual number of customers (can't use more tills than customers)
    if (static_cast<size_t>(n) > customers.size()) {
        n = static_cast<int>(customers.size());
    }
    
    // Use vector with bounds checking instead of raw arrays
    // Initialize all tills to 0 time
    std::vector<int> tills(n, 0);
    
    // Process each customer in queue order
    for (size_t i = 0; i < customers.size(); i++) {
        // Find the till with minimum time using standard algorithm
        // This is safe as we validated n > 0 and tills.size() == n
        auto minTill = std::min_element(tills.begin(), tills.end());
        
        // Check for integer overflow before addition
        if (*minTill > std::numeric_limits<int>::max() - customers[i]) {
            return std::numeric_limits<int>::max(); // Overflow would occur
        }
        
        // Assign customer to the till with minimum time
        *minTill += customers[i];
    }
    
    // Return the maximum time among all tills (when last customer finishes)
    return *std::max_element(tills.begin(), tills.end());
}

int main() {
    // Test case 1: Single till
    std::vector<int> test1 = {5, 3, 4};
    std::cout << "Test 1: " << queueTime(test1, 1) << " (expected 12)" << std::endl;
    
    // Test case 2: Two tills with varying times
    std::vector<int> test2 = {10, 2, 3, 3};
    std::cout << "Test 2: " << queueTime(test2, 2) << " (expected 10)" << std::endl;
    
    // Test case 3: Two tills
    std::vector<int> test3 = {2, 3, 10};
    std::cout << "Test 3: " << queueTime(test3, 2) << " (expected 12)" << std::endl;
    
    // Test case 4: Empty queue
    std::vector<int> test4 = {};
    std::cout << "Test 4: " << queueTime(test4, 2) << " (expected 0)" << std::endl;
    
    // Test case 5: More tills than customers
    std::vector<int> test5 = {1, 2, 3};
    std::cout << "Test 5: " << queueTime(test5, 5) << " (expected 3)" << std::endl;
    
    return 0;
}
