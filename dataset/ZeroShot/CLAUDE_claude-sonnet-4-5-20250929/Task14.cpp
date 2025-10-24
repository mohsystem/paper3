
#include <iostream>
#include <vector>

class Task14 {
public:
    static int findEqualSumIndex(const std::vector<int>& arr) {
        if (arr.empty()) {
            return -1;
        }
        
        // Calculate total sum of array
        long long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }
        
        // Iterate through array and check if left sum equals right sum
        long long leftSum = 0;
        for (size_t i = 0; i < arr.size(); i++) {
            // Right sum is total sum minus left sum minus current element
            long long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            // Add current element to left sum for next iteration
            leftSum += arr[i];
        }
        
        return -1;
    }
};

int main() {
    // Test case 1
    std::vector<int> test1 = {1, 2, 3, 4, 3, 2, 1};
    std::cout << "Test 1: " << Task14::findEqualSumIndex(test1) << std::endl; // Expected: 3
    
    // Test case 2
    std::vector<int> test2 = {1, 100, 50, -51, 1, 1};
    std::cout << "Test 2: " << Task14::findEqualSumIndex(test2) << std::endl; // Expected: 1
    
    // Test case 3
    std::vector<int> test3 = {20, 10, -80, 10, 10, 15, 35};
    std::cout << "Test 3: " << Task14::findEqualSumIndex(test3) << std::endl; // Expected: 0
    
    // Test case 4
    std::vector<int> test4 = {1, 2, 3};
    std::cout << "Test 4: " << Task14::findEqualSumIndex(test4) << std::endl; // Expected: -1
    
    // Test case 5
    std::vector<int> test5 = {10};
    std::cout << "Test 5: " << Task14::findEqualSumIndex(test5) << std::endl; // Expected: 0
    
    return 0;
}
