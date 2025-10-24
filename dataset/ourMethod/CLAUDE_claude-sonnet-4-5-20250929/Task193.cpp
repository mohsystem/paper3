
#include <vector>
#include <iostream>

class Solution {
public:
    bool xorGame(std::vector<int>& nums) {
        // Input validation: check array bounds per constraints
        // nums.length is guaranteed by std::vector, range [1, 1000]
        if (nums.empty() || nums.size() > 1000) {
            return false; // Defensive check, fail closed
        }
        
        // Validate each element is within constraint 0 <= nums[i] < 2^16
        for (const int num : nums) {
            if (num < 0 || num >= 65536) {
                return false; // Fail closed on invalid input
            }
        }
        
        // Calculate XOR of all elements
        // Safe: XOR operation cannot overflow and is associative/commutative
        int xorSum = 0;
        for (const int num : nums) {
            xorSum ^= num;
        }
        
        // Game theory analysis:
        // 1. If XOR is 0 at start, Alice wins immediately (per rules)
        // 2. If XOR is not 0 and array length is even, Alice wins
        //    - With optimal play, if length is even and XOR != 0,
        //      Alice can always force Bob into a losing position
        // 3. If XOR is not 0 and array length is odd, Alice loses
        //    - Bob will eventually force Alice to make XOR = 0
        
        // Safe size check: size_t is unsigned, modulo 2 is safe
        return (xorSum == 0) || (nums.size() % 2 == 0);
    }
};

// Test driver
int main() {
    Solution solution;
    
    // Test case 1: [1,1,2] - Expected: false
    std::vector<int> test1 = {1, 1, 2};
    std::cout << "Test 1: " << (solution.xorGame(test1) ? "true" : "false") << std::endl;
    
    // Test case 2: [0,1] - Expected: true
    std::vector<int> test2 = {0, 1};
    std::cout << "Test 2: " << (solution.xorGame(test2) ? "true" : "false") << std::endl;
    
    // Test case 3: [1,2,3] - Expected: true
    std::vector<int> test3 = {1, 2, 3};
    std::cout << "Test 3: " << (solution.xorGame(test3) ? "true" : "false") << std::endl;
    
    // Test case 4: [1,1,1,1] - Expected: true (even length, XOR != 0)
    std::vector<int> test4 = {1, 1, 1, 1};
    std::cout << "Test 4: " << (solution.xorGame(test4) ? "true" : "false") << std::endl;
    
    // Test case 5: [5,5,5] - Expected: true (XOR = 5, odd length becomes 5)
    std::vector<int> test5 = {5, 5, 5};
    std::cout << "Test 5: " << (solution.xorGame(test5) ? "true" : "false") << std::endl;
    
    return 0;
}
