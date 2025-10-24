
#include <iostream>
#include <vector>
#include <algorithm>

int longestRun(const std::vector<int>& arr) {
    if (arr.empty()) {
        return 0;
    }
    
    if (arr.size() == 1) {
        return 1;
    }
    
    int maxLength = 1;
    int currentLength = 1;
    
    for (size_t i = 1; i < arr.size(); i++) {
        if (arr[i] - arr[i - 1] == 1 || arr[i] - arr[i - 1] == -1) {
            currentLength++;
            maxLength = std::max(maxLength, currentLength);
        } else {
            currentLength = 1;
        }
    }
    
    return maxLength;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test 1: " << longestRun(test1) << " (Expected: 5)" << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {1, 2, 3, 10, 11, 15};
    std::cout << "Test 2: " << longestRun(test2) << " (Expected: 3)" << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {5, 4, 2, 1};
    std::cout << "Test 3: " << longestRun(test3) << " (Expected: 2)" << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {3, 5, 7, 10, 15};
    std::cout << "Test 4: " << longestRun(test4) << " (Expected: 1)" << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {10, 9, 8, 7, 6, 5, 4};
    std::cout << "Test 5: " << longestRun(test5) << " (Expected: 7)" << std::endl;
    
    return 0;
}
