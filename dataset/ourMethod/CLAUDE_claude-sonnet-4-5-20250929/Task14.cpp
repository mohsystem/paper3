
#include <iostream>
#include <vector>
#include <climits>
#include <stdexcept>

// Function to find equilibrium index where left sum equals right sum
// Returns the lowest index or -1 if not found
// Security: Input validation, overflow prevention, bounds checking
int findEquilibriumIndex(const std::vector<int>& arr) {
    // Input validation: Check array size is within constraints (0 < arr < 1000)
    if (arr.empty() || arr.size() >= 1000) {
        throw std::invalid_argument("Array size must be between 1 and 999");
    }
    
    int n = static_cast<int>(arr.size());
    
    // Calculate total sum with overflow detection
    // Using long long to prevent integer overflow during summation
    long long totalSum = 0;
    for (int i = 0; i < n; i++) {
        // Check for overflow before addition
        if ((totalSum > 0 && arr[i] > LLONG_MAX - totalSum) ||
            (totalSum < 0 && arr[i] < LLONG_MIN - totalSum)) {
            throw std::overflow_error("Sum calculation overflow");
        }
        totalSum += arr[i];
    }
    
    // Iterate through array to find equilibrium index
    long long leftSum = 0;
    
    for (int i = 0; i < n; i++) {
        // Right sum = total - left - current element
        long long rightSum = totalSum - leftSum - arr[i];
        
        // Check if left sum equals right sum
        if (leftSum == rightSum) {
            return i;  // Return lowest index where balance occurs
        }
        
        // Update left sum with overflow check
        if ((leftSum > 0 && arr[i] > LLONG_MAX - leftSum) ||
            (leftSum < 0 && arr[i] < LLONG_MIN - leftSum)) {
            throw std::overflow_error("Left sum calculation overflow");
        }
        leftSum += arr[i];
    }
    
    // No equilibrium index found
    return -1;
}

int main() {
    try {
        // Test case 1: Middle equilibrium
        std::vector<int> test1 = {1, 2, 3, 4, 3, 2, 1};
        std::cout << "Test 1: " << findEquilibriumIndex(test1) << " (Expected: 3)" << std::endl;
        
        // Test case 2: Early equilibrium
        std::vector<int> test2 = {1, 100, 50, -51, 1, 1};
        std::cout << "Test 2: " << findEquilibriumIndex(test2) << " (Expected: 1)" << std::endl;
        
        // Test case 3: Index 0 equilibrium
        std::vector<int> test3 = {20, 10, -80, 10, 10, 15, 35};
        std::cout << "Test 3: " << findEquilibriumIndex(test3) << " (Expected: 0)" << std::endl;
        
        // Test case 4: No equilibrium
        std::vector<int> test4 = {1, 2, 3, 4, 5};
        std::cout << "Test 4: " << findEquilibriumIndex(test4) << " (Expected: -1)" << std::endl;
        
        // Test case 5: Single element (always equilibrium at index 0)
        std::vector<int> test5 = {42};
        std::cout << "Test 5: " << findEquilibriumIndex(test5) << " (Expected: 0)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
