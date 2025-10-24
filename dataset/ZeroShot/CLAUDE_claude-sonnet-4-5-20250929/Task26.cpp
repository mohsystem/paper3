
#include <iostream>
#include <vector>
#include <unordered_map>

class Task26 {
public:
    static int findOdd(const std::vector<int>& array) {
        std::unordered_map<int, int> countMap;
        
        for (int num : array) {
            countMap[num]++;
        }
        
        for (const auto& pair : countMap) {
            if (pair.second % 2 != 0) {
                return pair.first;
            }
        }
        
        return 0;
    }
};

int main() {
    // Test case 1
    std::vector<int> test1 = {7};
    std::cout << "Test 1: " << Task26::findOdd(test1) << std::endl; // Expected: 7
    
    // Test case 2
    std::vector<int> test2 = {0};
    std::cout << "Test 2: " << Task26::findOdd(test2) << std::endl; // Expected: 0
    
    // Test case 3
    std::vector<int> test3 = {1, 1, 2};
    std::cout << "Test 3: " << Task26::findOdd(test3) << std::endl; // Expected: 2
    
    // Test case 4
    std::vector<int> test4 = {0, 1, 0, 1, 0};
    std::cout << "Test 4: " << Task26::findOdd(test4) << std::endl; // Expected: 0
    
    // Test case 5
    std::vector<int> test5 = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    std::cout << "Test 5: " << Task26::findOdd(test5) << std::endl; // Expected: 4
    
    return 0;
}
