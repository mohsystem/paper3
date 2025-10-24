
#include <iostream>
#include <string>
#include <algorithm>

std::string reverseString(std::string input) {
    std::reverse(input.begin(), input.end());
    return input;
}

int main() {
    // Test case 1
    std::string test1 = "Hello";
    std::cout << "Input: " << test1 << std::endl;
    std::cout << "Output: " << reverseString(test1) << std::endl;
    std::cout << std::endl;
    
    // Test case 2
    std::string test2 = "12345";
    std::cout << "Input: " << test2 << std::endl;
    std::cout << "Output: " << reverseString(test2) << std::endl;
    std::cout << std::endl;
    
    // Test case 3
    std::string test3 = "A man a plan a canal Panama";
    std::cout << "Input: " << test3 << std::endl;
    std::cout << "Output: " << reverseString(test3) << std::endl;
    std::cout << std::endl;
    
    // Test case 4
    std::string test4 = "";
    std::cout << "Input: \\"" << test4 << "\\"" << std::endl;
    std::cout << "Output: \\"" << reverseString(test4) << "\\"" << std::endl;
    std::cout << std::endl;
    
    // Test case 5
    std::string test5 = "Reverse Me!";
    std::cout << "Input: " << test5 << std::endl;
    std::cout << "Output: " << reverseString(test5) << std::endl;
    
    return 0;
}
