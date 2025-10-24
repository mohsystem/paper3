
#include <iostream>
#include <vector>
#include <stdexcept>

class Task18 {
public:
    static int binaryArrayToInt(const std::vector<int>& binaryArray) {
        // Security: Check for empty array
        if (binaryArray.empty()) {
            return 0;
        }
        
        // Security: Validate array length to prevent integer overflow
        if (binaryArray.size() > 31) {
            throw std::invalid_argument("Binary array too long, may cause integer overflow");
        }
        
        int result = 0;
        
        // Security: Validate each element and convert binary to integer
        for (size_t i = 0; i < binaryArray.size(); i++) {
            // Security: Ensure only 0 or 1 values are present
            if (binaryArray[i] != 0 && binaryArray[i] != 1) {
                throw std::invalid_argument("Array must contain only 0 or 1 values");
            }
            
            // Shift left and add current bit
            result = (result << 1) | binaryArray[i];
        }
        
        return result;
    }
};

int main() {
    // Test case 1
    std::vector<int> test1 = {0, 0, 0, 1};
    std::cout << "Test 1: [0, 0, 0, 1] => " << Task18::binaryArrayToInt(test1) << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {0, 0, 1, 0};
    std::cout << "Test 2: [0, 0, 1, 0] => " << Task18::binaryArrayToInt(test2) << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {0, 1, 0, 1};
    std::cout << "Test 3: [0, 1, 0, 1] => " << Task18::binaryArrayToInt(test3) << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {1, 0, 0, 1};
    std::cout << "Test 4: [1, 0, 0, 1] => " << Task18::binaryArrayToInt(test4) << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {1, 1, 1, 1};
    std::cout << "Test 5: [1, 1, 1, 1] => " << Task18::binaryArrayToInt(test5) << std::endl;
    
    return 0;
}
