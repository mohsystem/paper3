
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>

class Task145 {
public:
    /**
     * Finds the contiguous subarray with maximum sum using Kadane's algorithm\n     * @param arr input vector of integers\n     * @return maximum sum of contiguous subarray\n     */\n    static int maxSubarraySum(const std::vector<int>& arr) {\n        // Input validation\n        if (arr.empty()) {\n            throw std::invalid_argument("Array cannot be empty");\n        }\n        \n        int maxSum = arr[0];\n        int currentSum = arr[0];\n        \n        // Kadane's algorithm
        for (size_t i = 1; i < arr.size(); i++) {
            currentSum = std::max(arr[i], currentSum + arr[i]);
            maxSum = std::max(maxSum, currentSum);
        }
        
        return maxSum;
    }
    
    static void printArray(const std::vector<int>& arr) {
        std::cout << "[";
        for (size_t i = 0; i < arr.size(); i++) {
            std::cout << arr[i];
            if (i < arr.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
};

int main() {
    // Test case 1: Mixed positive and negative numbers
    std::vector<int> test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    std::cout << "Test 1: ";
    Task145::printArray(test1);
    std::cout << "Maximum subarray sum: " << Task145::maxSubarraySum(test1) << std::endl;
    std::cout << std::endl;
    
    // Test case 2: All negative numbers
    std::vector<int> test2 = {-5, -2, -8, -1, -4};
    std::cout << "Test 2: ";
    Task145::printArray(test2);
    std::cout << "Maximum subarray sum: " << Task145::maxSubarraySum(test2) << std::endl;
    std::cout << std::endl;
    
    // Test case 3: All positive numbers
    std::vector<int> test3 = {1, 2, 3, 4, 5};
    std::cout << "Test 3: ";
    Task145::printArray(test3);
    std::cout << "Maximum subarray sum: " << Task145::maxSubarraySum(test3) << std::endl;
    std::cout << std::endl;
    
    // Test case 4: Single element
    std::vector<int> test4 = {10};
    std::cout << "Test 4: ";
    Task145::printArray(test4);
    std::cout << "Maximum subarray sum: " << Task145::maxSubarraySum(test4) << std::endl;
    std::cout << std::endl;
    
    // Test case 5: Large numbers
    std::vector<int> test5 = {-1, -2, 5, -3, 4, 6, -2};
    std::cout << "Test 5: ";
    Task145::printArray(test5);
    std::cout << "Maximum subarray sum: " << Task145::maxSubarraySum(test5) << std::endl;
    
    return 0;
}
