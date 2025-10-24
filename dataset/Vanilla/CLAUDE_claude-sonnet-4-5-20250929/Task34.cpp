
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
    std::cout << "Test 1: isNarcissistic(153) = " << (isNarcissistic(153) ? "true" : "false") << std::endl;
    std::cout << "Test 2: isNarcissistic(1652) = " << (isNarcissistic(1652) ? "true" : "false") << std::endl;
    std::cout << "Test 3: isNarcissistic(9) = " << (isNarcissistic(9) ? "true" : "false") << std::endl;
    std::cout << "Test 4: isNarcissistic(9474) = " << (isNarcissistic(9474) ? "true" : "false") << std::endl;
    std::cout << "Test 5: isNarcissistic(1634) = " << (isNarcissistic(1634) ? "true" : "false") << std::endl;
    
    return 0;
}
