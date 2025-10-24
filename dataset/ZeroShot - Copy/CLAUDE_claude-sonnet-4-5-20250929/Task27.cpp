
#include <iostream>
#include <vector>
#include <algorithm>

int findOutlier(const std::vector<int>& integers) {
    int evenCount = 0;
    int oddCount = 0;
    int lastEven = 0;
    int lastOdd = 0;
    
    // Check first 3 elements to determine the majority
    for (int i = 0; i < std::min(3, (int)integers.size()); i++) {
        if (integers[i] % 2 == 0) {
            evenCount++;
            lastEven = integers[i];
        } else {
            oddCount++;
            lastOdd = integers[i];
        }
    }
    
    // Determine if we're looking for odd or even outlier
    bool lookingForOdd = evenCount > oddCount;
    
    // If we already found the outlier in first 3 elements
    if (lookingForOdd && oddCount == 1) {
        return lastOdd;
    }
    if (!lookingForOdd && evenCount == 1) {
        return lastEven;
    }
    
    // Search the rest of the array
    for (int i = 3; i < integers.size(); i++) {
        if (lookingForOdd && integers[i] % 2 != 0) {
            return integers[i];
        }
        if (!lookingForOdd && integers[i] % 2 == 0) {
            return integers[i];
        }
    }
    
    return 0; // Should never reach here with valid input
}

int main() {
    // Test case 1
    std::vector<int> test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
    std::cout << "Test 1: " << findOutlier(test1) << std::endl; // Expected: 11
    
    // Test case 2
    std::vector<int> test2 = {160, 3, 1719, 19, 11, 13, -21};
    std::cout << "Test 2: " << findOutlier(test2) << std::endl; // Expected: 160
    
    // Test case 3
    std::vector<int> test3 = {1, 3, 5, 7, 9, 10};
    std::cout << "Test 3: " << findOutlier(test3) << std::endl; // Expected: 10
    
    // Test case 4
    std::vector<int> test4 = {2, 4, 6, 8, 10, 1};
    std::cout << "Test 4: " << findOutlier(test4) << std::endl; // Expected: 1
    
    // Test case 5
    std::vector<int> test5 = {-3, -5, -7, 2, -9};
    std::cout << "Test 5: " << findOutlier(test5) << std::endl; // Expected: 2
    
    return 0;
}
