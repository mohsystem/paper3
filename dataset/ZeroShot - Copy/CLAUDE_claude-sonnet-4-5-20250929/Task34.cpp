
#include <iostream>
#include <string>
#include <cmath>

bool isNarcissistic(int number) {
    std::string numStr = std::to_string(number);
    int numDigits = numStr.length();
    int sum = 0;
    
    for (char digit : numStr) {
        int digitValue = digit - '0';
        sum += std::pow(digitValue, numDigits);
    }
    
    return sum == number;
}

int main() {
    // Test cases
    std::cout << "Test 1 - Input: 153, Output: " << (isNarcissistic(153) ? "true" : "false") << std::endl;
    std::cout << "Test 2 - Input: 1652, Output: " << (isNarcissistic(1652) ? "true" : "false") << std::endl;
    std::cout << "Test 3 - Input: 9474, Output: " << (isNarcissistic(9474) ? "true" : "false") << std::endl;
    std::cout << "Test 4 - Input: 9475, Output: " << (isNarcissistic(9475) ? "true" : "false") << std::endl;
    std::cout << "Test 5 - Input: 9, Output: " << (isNarcissistic(9) ? "true" : "false") << std::endl;
    
    return 0;
}
