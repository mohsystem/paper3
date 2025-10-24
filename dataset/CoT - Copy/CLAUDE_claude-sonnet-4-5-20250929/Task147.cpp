
#include <iostream>
#include <vector>
#include <queue>
#include <stdexcept>

class Task147 {
public:
    // Method to find kth largest element using min heap
    static int findKthLargest(const std::vector<int>& nums, int k) {
        // Input validation
        if (nums.empty()) {
            throw std::invalid_argument("Array cannot be empty");
        }
        if (k <= 0 || k > static_cast<int>(nums.size())) {
            throw std::invalid_argument("k must be between 1 and array length");
        }
        
        // Use min heap of size k to maintain k largest elements
        std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
        
        for (int num : nums) {
            minHeap.push(num);
            if (static_cast<int>(minHeap.size()) > k) {
                minHeap.pop();
            }
        }
        
        return minHeap.top();
    }
};

int main() {
    // Test case 1: Normal case
    std::vector<int> test1 = {3, 2, 1, 5, 6, 4};
    int k1 = 2;
    std::cout << "Test 1: Array: [3, 2, 1, 5, 6, 4], k=" << k1 << std::endl;
    std::cout << "Result: " << Task147::findKthLargest(test1, k1) << std::endl;
    std::cout << std::endl;
    
    // Test case 2: Array with duplicates
    std::vector<int> test2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int k2 = 4;
    std::cout << "Test 2: Array: [3, 2, 3, 1, 2, 4, 5, 5, 6], k=" << k2 << std::endl;
    std::cout << "Result: " << Task147::findKthLargest(test2, k2) << std::endl;
    std::cout << std::endl;
    
    // Test case 3: k = 1 (largest element)
    std::vector<int> test3 = {7, 10, 4, 3, 20, 15};
    int k3 = 1;
    std::cout << "Test 3: Array: [7, 10, 4, 3, 20, 15], k=" << k3 << std::endl;
    std::cout << "Result: " << Task147::findKthLargest(test3, k3) << std::endl;
    std::cout << std::endl;
    
    // Test case 4: k equals array length (smallest element)
    std::vector<int> test4 = {5, 2, 8, 1, 9};
    int k4 = 5;
    std::cout << "Test 4: Array: [5, 2, 8, 1, 9], k=" << k4 << std::endl;
    std::cout << "Result: " << Task147::findKthLargest(test4, k4) << std::endl;
    std::cout << std::endl;
    
    // Test case 5: Single element array
    std::vector<int> test5 = {42};
    int k5 = 1;
    std::cout << "Test 5: Array: [42], k=" << k5 << std::endl;
    std::cout << "Result: " << Task147::findKthLargest(test5, k5) << std::endl;
    
    return 0;
}
