
#include <vector>
#include <iostream>
#include <stdexcept>

class Task192 {
public:
    // Calculate XOR of all elements in the array
    // Returns the XOR result
    static int calculateXOR(const std::vector<int>& nums) {
        int xorResult = 0;
        for (int num : nums) {
            xorResult ^= num;
        }
        return xorResult;
    }
    
    // Determine if Alice wins the game
    // Input validation ensures array length is within bounds
    // Returns true if Alice wins, false otherwise
    static bool xorGame(const std::vector<int>& nums) {
        // Input validation: check array is not empty and within constraints
        if (nums.empty() || nums.size() > 1000) {
            throw std::invalid_argument("Array size must be between 1 and 1000");
        }
        
        // Validate each element is within bounds [0, 2^16)
        for (int num : nums) {
            if (num < 0 || num >= 65536) {
                throw std::invalid_argument("Array elements must be in range [0, 65536)");
            }
        }
        
        // Calculate initial XOR of all elements
        int xorResult = calculateXOR(nums);
        
        // If XOR is 0 at start, Alice wins immediately
        if (xorResult == 0) {
            return true;
        }
        
        // If array length is even, Alice wins
        // Mathematical proof: if XOR != 0 and length is even,
        // Alice can always mirror Bob's moves to avoid losing
        return nums.size() % 2 == 0;
    }
};

int main() {
    // Test case 1: [1,1,2] -> false
    try {
        std::vector<int> test1 = {1, 1, 2};
        bool result1 = Task192::xorGame(test1);
        std::cout << "Test 1: " << (result1 ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: [0,1] -> true
    try {
        std::vector<int> test2 = {0, 1};
        bool result2 = Task192::xorGame(test2);
        std::cout << "Test 2: " << (result2 ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: [1,2,3] -> true
    try {
        std::vector<int> test3 = {1, 2, 3};
        bool result3 = Task192::xorGame(test3);
        std::cout << "Test 3: " << (result3 ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: [5,5,5,5] -> true (even length, XOR != 0)
    try {
        std::vector<int> test4 = {5, 5, 5, 5};
        bool result4 = Task192::xorGame(test4);
        std::cout << "Test 4: " << (result4 ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: [1] -> true (XOR of single element is itself, != 0, odd length)
    try {
        std::vector<int> test5 = {1};
        bool result5 = Task192::xorGame(test5);
        std::cout << "Test 5: " << (result5 ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
