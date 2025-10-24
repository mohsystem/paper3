
#include <iostream>
#include <vector>
#include <string>

std::string oddOrEven(const std::vector<int>& array) {
    if (array.empty()) {
        return "even";
    }
    
    int sum = 0;
    for (int num : array) {
        sum += num;
    }
    
    return (sum % 2 == 0) ? "even" : "odd";
}

int main() {
    // Test case 1
    std::vector<int> test1 = {0};
    std::cout << "Test 1: " << oddOrEven(test1) << std::endl;
    
    // Test case 2
    std::vector<int> test2 = {0, 1, 4};
    std::cout << "Test 2: " << oddOrEven(test2) << std::endl;
    
    // Test case 3
    std::vector<int> test3 = {0, -1, -5};
    std::cout << "Test 3: " << oddOrEven(test3) << std::endl;
    
    // Test case 4
    std::vector<int> test4 = {};
    std::cout << "Test 4: " << oddOrEven(test4) << std::endl;
    
    // Test case 5
    std::vector<int> test5 = {2, 4, 6, 8};
    std::cout << "Test 5: " << oddOrEven(test5) << std::endl;
    
    return 0;
}
