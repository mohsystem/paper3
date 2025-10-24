
#include <iostream>
#include <vector>

int binaryArrayToNumber(const std::vector<int>& binary) {
    if (binary.empty()) {
        return 0;
    }
    
    int result = 0;
    for (size_t i = 0; i < binary.size(); i++) {
        result = result * 2 + binary[i];
    }
    return result;
}

int main() {
    // Test case 1
    std::vector<int> test1 = {0, 0, 0, 1};
    std::cout << "Testing: [0, 0, 0, 1] ==> " << binaryArrayToNumber(test1) << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {0, 0, 1, 0};
    std::cout << "Testing: [0, 0, 1, 0] ==> " << binaryArrayToNumber(test2) << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {0, 1, 0, 1};
    std::cout << "Testing: [0, 1, 0, 1] ==> " << binaryArrayToNumber(test3) << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {1, 0, 0, 1};
    std::cout << "Testing: [1, 0, 0, 1] ==> " << binaryArrayToNumber(test4) << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {1, 1, 1, 1};
    std::cout << "Testing: [1, 1, 1, 1] ==> " << binaryArrayToNumber(test5) << std::endl;
    
    return 0;
}
