
#include <iostream>
#include <vector>
#include <climits>

int longestRun(const std::vector<int>& arr) {
    // Input validation
    if (arr.empty()) {
        return 0;
    }
    
    if (arr.size() == 1) {
        return 1;
    }
    
    int maxLength = 1;
    int currentLength = 1;
    
    for (size_t i = 1; i < arr.size(); i++) {
        // Check for overflow when computing difference
        long long diff = static_cast<long long>(arr[i]) - static_cast<long long>(arr[i-1]);
        
        // Check if consecutive (increasing or decreasing by 1)
        if (diff == 1 || diff == -1) {
            currentLength++;
            if (currentLength > maxLength) {
                maxLength = currentLength;
            }
        } else {
            currentLength = 1;
        }
    }
    
    return maxLength;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {1, 2, 3, 5, 6, 7, 8, 9};
    std::cout << "Test 1: " << longestRun(test1) << std::endl; // Expected: 5
    
    // Test case 2
    std::vector<int> test2 = {1, 2, 3, 10, 11, 15};
    std::cout << "Test 2: " << longestRun(test2) << std::endl; // Expected: 3
    
    // Test case 3
    std::vector<int> test3 = {5, 4, 2, 1};
    std::cout << "Test 3: " << longestRun(test3) << std::endl; // Expected: 2
    
    // Test case 4
    std::vector<int> test4 = {3, 5, 7, 10, 15};
    std::cout << "Test 4: " << longestRun(test4) << std::endl; // Expected: 1
    
    // Test case 5
    std::vector<int> test5 = {10, 9, 8, 7, 6, 5};
    std::cout << "Test 5: " << longestRun(test5) << std::endl; // Expected: 6
    
    return 0;
}
