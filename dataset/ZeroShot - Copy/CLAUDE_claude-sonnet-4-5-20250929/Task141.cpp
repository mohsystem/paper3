
#include <iostream>
#include <string>
#include <algorithm>

std::string reverseString(std::string input) {
    std::reverse(input.begin(), input.end());
    return input;
}

int main() {
    // Test case 1: Normal string
    std::cout << "Test 1: " << reverseString("hello") << std::endl;
    
    // Test case 2: String with spaces
    std::cout << "Test 2: " << reverseString("hello world") << std::endl;
    
    // Test case 3: Empty string
    std::cout << "Test 3: " << reverseString("") << std::endl;
    
    // Test case 4: Single character
    std::cout << "Test 4: " << reverseString("a") << std::endl;
    
    // Test case 5: String with special characters
    std::cout << "Test 5: " << reverseString("Hello123!@#") << std::endl;
    
    return 0;
}
