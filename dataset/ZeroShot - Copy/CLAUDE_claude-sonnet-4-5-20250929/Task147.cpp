
#include <iostream>
#include <vector>
#include <queue>
#include <stdexcept>

int findKthLargest(const std::vector<int>& nums, int k) {
    if (nums.empty() || k <= 0 || k > nums.size()) {
        throw std::invalid_argument("Invalid input");
    }
    
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
    
    for (int num : nums) {
        minHeap.push(num);
        if (minHeap.size() > static_cast<size_t>(k)) {
            minHeap.pop();
        }
    }
    
    return minHeap.top();
}

int main() {
    // Test case 1
    std::vector<int> arr1 = {3, 2, 1, 5, 6, 4};
    int k1 = 2;
    std::cout << "Test 1: " << findKthLargest(arr1, k1) << " (Expected: 5)" << std::endl;
    
    // Test case 2
    std::vector<int> arr2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int k2 = 4;
    std::cout << "Test 2: " << findKthLargest(arr2, k2) << " (Expected: 4)" << std::endl;
    
    // Test case 3
    std::vector<int> arr3 = {1};
    int k3 = 1;
    std::cout << "Test 3: " << findKthLargest(arr3, k3) << " (Expected: 1)" << std::endl;
    
    // Test case 4
    std::vector<int> arr4 = {7, 10, 4, 3, 20, 15};
    int k4 = 3;
    std::cout << "Test 4: " << findKthLargest(arr4, k4) << " (Expected: 10)" << std::endl;
    
    // Test case 5
    std::vector<int> arr5 = {-1, -5, -3, -2, -4};
    int k5 = 2;
    std::cout << "Test 5: " << findKthLargest(arr5, k5) << " (Expected: -2)" << std::endl;
    
    return 0;
}
