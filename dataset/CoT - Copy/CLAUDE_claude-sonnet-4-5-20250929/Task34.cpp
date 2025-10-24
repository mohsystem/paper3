
#include <iostream>
#include <cmath>
#include <string>

bool isNarcissistic(int number) {
    // Input validation - ensure positive integer
    if (number < 0) {
        return false;
    }
    
    // Convert to string to get digits
    std::string numStr = std::to_string(number);
    int numDigits = numStr.length();
    
    // Calculate sum of digits raised to power of digit count
    int sum = 0;
    for (char digitChar : numStr) {
        int digit = digitChar - '0';
        sum += static_cast<int>(std::pow(digit, numDigits));
    }
    
    return sum == number;
}

int main() {
    // Test cases
    std::cout << "Test 1 - 153: " << (isNarcissistic(153) ? "true" : "false") << std::endl;
    std::cout << "Test 2 - 1652: " << (isNarcissistic(1652) ? "true" : "false") << std::endl;
    std::cout << "Test 3 - 9: " << (isNarcissistic(9) ? "true" : "false") << std::endl;
    std::cout << "Test 4 - 9474: " << (isNarcissistic(9474) ? "true" : "false") << std::endl;
    std::cout << "Test 5 - 370: " << (isNarcissistic(370) ? "true" : "false") << std::endl;
    
    return 0;
}
