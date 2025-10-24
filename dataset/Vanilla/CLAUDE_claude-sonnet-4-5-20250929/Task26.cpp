
#include <iostream>
#include <unordered_map>
#include <vector>

int findOdd(const std::vector<int>& arr) {
    std::unordered_map<int, int> countMap;
    
    for (int num : arr) {
        countMap[num]++;
    }
    
    for (const auto& pair : countMap) {
        if (pair.second % 2 != 0) {
            return pair.first;
        }
    }
    
    return 0;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {7};
    std::cout << "Test 1: " << findOdd(test1) << std::endl; // Expected: 7
    
    // Test case 2
    std::vector<int> test2 = {0};
    std::cout << "Test 2: " << findOdd(test2) << std::endl; // Expected: 0
    
    // Test case 3
    std::vector<int> test3 = {1, 1, 2};
    std::cout << "Test 3: " << findOdd(test3) << std::endl; // Expected: 2
    
    // Test case 4
    std::vector<int> test4 = {0, 1, 0, 1, 0};
    std::cout << "Test 4: " << findOdd(test4) << std::endl; // Expected: 0
    
    // Test case 5
    std::vector<int> test5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    std::cout << "Test 5: " << findOdd(test5) << std::endl; // Expected: 4
    
    return 0;
}
