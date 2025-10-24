
#include <iostream>
#include <vector>
#include <stdexcept>

class Task27 {
public:
    static int findOutlier(const std::vector<int>& array) {
        // Security: Validate input to prevent errors
        if (array.size() < 3) {
            throw std::invalid_argument("Array must have at least 3 elements");
        }
        
        // Count even and odd numbers in first 3 elements to determine the pattern
        int evenCount = 0;
        int oddCount = 0;
        
        // Check first 3 elements to determine if we're looking for even or odd outlier\n        for (int i = 0; i < 3; i++) {\n            if (array[i] % 2 == 0) {\n                evenCount++;\n            } else {\n                oddCount++;\n            }\n        }\n        \n        // Determine if we're looking for the odd or even outlier
        bool lookingForOdd = evenCount > oddCount;
        
        // Find and return the outlier
        for (int num : array) {
            if (lookingForOdd && num % 2 != 0) {
                return num;
            } else if (!lookingForOdd && num % 2 == 0) {
                return num;
            }
        }
        
        // This should never be reached given valid input
        throw std::runtime_error("No outlier found in array");
    }
};

int main() {
    // Test case 1: Only odd number
    std::vector<int> test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
    std::cout << "Test 1: " << Task27::findOutlier(test1) << std::endl; // Expected: 11
    
    // Test case 2: Only even number
    std::vector<int> test2 = {160, 3, 1719, 19, 11, 13, -21};
    std::cout << "Test 2: " << Task27::findOutlier(test2) << std::endl; // Expected: 160
    
    // Test case 3: Negative odd outlier
    std::vector<int> test3 = {2, 4, 6, 8, 10, -1};
    std::cout << "Test 3: " << Task27::findOutlier(test3) << std::endl; // Expected: -1
    
    // Test case 4: Negative even outlier
    std::vector<int> test4 = {1, 3, 5, 7, -2, 9};
    std::cout << "Test 4: " << Task27::findOutlier(test4) << std::endl; // Expected: -2
    
    // Test case 5: Zero as outlier
    std::vector<int> test5 = {1, 3, 5, 0, 7, 9};
    std::cout << "Test 5: " << Task27::findOutlier(test5) << std::endl; // Expected: 0
    
    return 0;
}
