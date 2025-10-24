
#include <iostream>
#include <vector>
#include <stdexcept>

class Task8 {
public:
    static char findMissingLetter(const std::vector<char>& array) {
        if (array.size() < 2) {
            throw std::invalid_argument("Invalid input array");
        }
        
        for (size_t i = 0; i < array.size() - 1; i++) {
            if (array[i + 1] - array[i] != 1) {
                return static_cast<char>(array[i] + 1);
            }
        }
        
        throw std::invalid_argument("No missing letter found");
    }
};

int main() {
    // Test case 1
    std::vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
    std::cout << "Test 1: " << Task8::findMissingLetter(test1) << std::endl; // Expected: e
    
    // Test case 2
    std::vector<char> test2 = {'O', 'Q', 'R', 'S'};
    std::cout << "Test 2: " << Task8::findMissingLetter(test2) << std::endl; // Expected: P
    
    // Test case 3
    std::vector<char> test3 = {'a', 'b', 'd'};
    std::cout << "Test 3: " << Task8::findMissingLetter(test3) << std::endl; // Expected: c
    
    // Test case 4
    std::vector<char> test4 = {'m', 'n', 'o', 'q', 'r'};
    std::cout << "Test 4: " << Task8::findMissingLetter(test4) << std::endl; // Expected: p
    
    // Test case 5
    std::vector<char> test5 = {'X', 'Z'};
    std::cout << "Test 5: " << Task8::findMissingLetter(test5) << std::endl; // Expected: Y
    
    return 0;
}
