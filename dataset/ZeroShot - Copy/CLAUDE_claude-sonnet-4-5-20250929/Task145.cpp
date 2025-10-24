
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

int maxSubarraySum(const std::vector<int>& arr) {
    if (arr.empty()) {
        return 0;
    }
    
    int maxSoFar = arr[0];
    int maxEndingHere = arr[0];
    
    for (size_t i = 1; i < arr.size(); i++) {
        maxEndingHere = std::max(arr[i], maxEndingHere + arr[i]);
        maxSoFar = std::max(maxSoFar, maxEndingHere);
    }
    
    return maxSoFar;
}

int main() {
    // Test case 1: Mix of positive and negative numbers
    std::vector<int> test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    std::cout << "Test 1: " << maxSubarraySum(test1) << std::endl; // Expected: 6
    
    // Test case 2: All negative numbers
    std::vector<int> test2 = {-5, -2, -8, -1, -4};
    std::cout << "Test 2: " << maxSubarraySum(test2) << std::endl; // Expected: -1
    
    // Test case 3: All positive numbers
    std::vector<int> test3 = {1, 2, 3, 4, 5};
    std::cout << "Test 3: " << maxSubarraySum(test3) << std::endl; // Expected: 15
    
    // Test case 4: Single element
    std::vector<int> test4 = {10};
    std::cout << "Test 4: " << maxSubarraySum(test4) << std::endl; // Expected: 10
    
    // Test case 5: Mix with zeros
    std::vector<int> test5 = {-2, 0, -1, 3, -2, 5, -3};
    std::cout << "Test 5: " << maxSubarraySum(test5) << std::endl; // Expected: 6
    
    return 0;
}
