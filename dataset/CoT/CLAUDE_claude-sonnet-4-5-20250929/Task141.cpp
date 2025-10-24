
#include <iostream>
#include <string>
#include <algorithm>

/**
 * Reverses a given string safely
 * @param input the string to reverse
 * @return the reversed string
 */
std::string reverseString(const std::string& input) {
    // Security: Pass by const reference to avoid unnecessary copying
    // and prevent modification of original string
    std::string reversed = input;
    
    // Use standard library function for safe reversal
    std::reverse(reversed.begin(), reversed.end());
    
    return reversed;
}

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::string test1 = "hello";
    std::cout << "Input: " << test1 << std::endl;
    std::cout << "Output: " << reverseString(test1) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    std::string test2 = "Java Programming";
    std::cout << "Input: " << test2 << std::endl;
    std::cout << "Output: " << reverseString(test2) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    std::string test3 = "12345";
    std::cout << "Input: " << test3 << std::endl;
    std::cout << "Output: " << reverseString(test3) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    std::string test4 = "";
    std::cout << "Input: (empty string)" << std::endl;
    std::cout << "Output: " << reverseString(test4) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    std::string test5 = "A";
    std::cout << "Input: " << test5 << std::endl;
    std::cout << "Output: " << reverseString(test5) << std::endl;
    
    return 0;
}
