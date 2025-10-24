
#include <iostream>
#include <vector>

class Task14 {
public:
    static int findEqualIndex(const std::vector<int>& arr) {
        // Input validation to prevent errors
        if (arr.empty()) {
            return -1;
        }
        
        // Calculate total sum to avoid repeated calculations
        long long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }
        
        // Track left sum as we iterate
        long long leftSum = 0;
        
        // Iterate through array to find equilibrium index
        for (size_t i = 0; i < arr.size(); i++) {
            // Right sum = total - left - current element
            long long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return static_cast<int>(i);
            }
            
            // Update left sum for next iteration
            leftSum += arr[i];
        }
        
        return -1;
    }
};

int main() {
    // Test case 1
    std::vector<int> test1 = {1, 2, 3, 4, 3, 2, 1};
    std::cout << "Test 1: " << Task14::findEqualIndex(test1) << std::endl; // Expected: 3
    
    // Test case 2
    std::vector<int> test2 = {1, 100, 50, -51, 1, 1};
    std::cout << "Test 2: " << Task14::findEqualIndex(test2) << std::endl; // Expected: 1
    
    // Test case 3
    std::vector<int> test3 = {20, 10, -80, 10, 10, 15, 35};
    std::cout << "Test 3: " << Task14::findEqualIndex(test3) << std::endl; // Expected: 0
    
    // Test case 4
    std::vector<int> test4 = {1, 2, 3};
    std::cout << "Test 4: " << Task14::findEqualIndex(test4) << std::endl; // Expected: -1
    
    // Test case 5
    std::vector<int> test5 = {0, 0, 0, 0};
    std::cout << "Test 5: " << Task14::findEqualIndex(test5) << std::endl; // Expected: 0
    
    return 0;
}
