
#include <iostream>
#include <vector>

class Task193 {
public:
    static bool xorGame(std::vector<int>& nums) {
        if (nums.empty()) {
            return false;
        }
        
        int xorResult = 0;
        for (int num : nums) {
            xorResult ^= num;
        }
        
        return xorResult == 0 || nums.size() % 2 == 0;
    }
};

int main() {
    // Test case 1
    std::vector<int> test1 = {1, 1, 2};
    std::cout << (Task193::xorGame(test1) ? "true" : "false") << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {0, 1};
    std::cout << (Task193::xorGame(test2) ? "true" : "false") << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {1, 2, 3};
    std::cout << (Task193::xorGame(test3) ? "true" : "false") << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {1};
    std::cout << (Task193::xorGame(test4) ? "true" : "false") << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {2, 2, 2, 2};
    std::cout << (Task193::xorGame(test5) ? "true" : "false") << std::endl;
    
    return 0;
}
