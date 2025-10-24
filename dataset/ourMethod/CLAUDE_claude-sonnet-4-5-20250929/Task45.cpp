
#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>
#include <cstring>
#include <vector>

// Maximum input length to prevent excessive memory allocation
const size_t MAX_INPUT_LENGTH = 1024;

// Safe integer input function with overflow protection
bool safeReadInteger(int& value) {
    // Clear any error flags
    std::cin.clear();
    
    // Read input as string first to validate
    std::string input;
    if (!std::getline(std::cin, input)) {
        return false;
    }
    
    // Validate input length
    if (input.empty() || input.length() > 20) {
        return false;
    }
    
    // Validate input contains only digits and optional leading sign
    size_t startPos = 0;
    if (input[0] == '-' || input[0] == '+') {
        startPos = 1;
    }
    
    for (size_t i = startPos; i < input.length(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) {
            return false;
        }
    }
    
    // Convert to integer with overflow checking
    try {
        size_t pos = 0;
        long long temp = std::stoll(input, &pos);
        
        // Check if entire string was consumed
        if (pos != input.length()) {
            return false;
        }
        
        // Check for overflow against int limits
        if (temp > std::numeric_limits<int>::max() || 
            temp < std::numeric_limits<int>::min()) {
            return false;
        }
        
        value = static_cast<int>(temp);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Safe string input with length validation
bool safeReadString(std::string& str, size_t maxLength = MAX_INPUT_LENGTH) {
    str.clear();
    
    if (!std::getline(std::cin, str)) {
        return false;
    }
    
    // Validate length to prevent excessive memory usage
    if (str.length() > maxLength) {
        str.clear();
        return false;
    }
    
    return true;
}

// Safe division operation with divide-by-zero check
bool safeDivide(int numerator, int denominator, int& result) {
    // Check for division by zero
    if (denominator == 0) {
        return false;
    }
    
    // Check for integer overflow (INT_MIN / -1 causes overflow)
    if (numerator == std::numeric_limits<int>::min() && denominator == -1) {
        return false;
    }
    
    result = numerator / denominator;
    return true;
}

// Safe multiplication with overflow detection
bool safeMultiply(int a, int b, int& result) {
    // Check for multiplication overflow
    if (a > 0 && b > 0 && a > std::numeric_limits<int>::max() / b) {
        return false;
    }
    if (a > 0 && b < 0 && b < std::numeric_limits<int>::min() / a) {
        return false;
    }
    if (a < 0 && b > 0 && a < std::numeric_limits<int>::min() / b) {
        return false;
    }
    if (a < 0 && b < 0 && a < std::numeric_limits<int>::max() / b) {
        return false;
    }
    
    result = a * b;
    return true;
}

// Safe addition with overflow detection
bool safeAdd(int a, int b, int& result) {
    // Check for addition overflow
    if (b > 0 && a > std::numeric_limits<int>::max() - b) {
        return false;
    }
    if (b < 0 && a < std::numeric_limits<int>::min() - b) {
        return false;
    }
    
    result = a + b;
    return true;
}

// Process operation with comprehensive error handling
bool processOperation(const std::string& operation, int operand1, int operand2, int& result) {
    // Validate operation string
    if (operation.empty() || operation.length() > 10) {
        return false;
    }
    
    // Perform operation based on validated input
    if (operation == "add") {
        return safeAdd(operand1, operand2, result);
    } else if (operation == "multiply") {
        return safeMultiply(operand1, operand2, result);
    } else if (operation == "divide") {
        return safeDivide(operand1, operand2, result);
    } else {
        return false;
    }
}

int main() {
    try {
        // Test case 1: Valid addition
        std::cout << "Test 1: Addition (10 + 20)" << std::endl;
        int result1 = 0;
        if (processOperation("add", 10, 20, result1)) {
            std::cout << "Result: " << result1 << std::endl;
        } else {
            std::cout << "Error: Operation failed" << std::endl;
        }
        
        // Test case 2: Valid multiplication
        std::cout << "\\nTest 2: Multiplication (5 * 6)" << std::endl;
        int result2 = 0;
        if (processOperation("multiply", 5, 6, result2)) {
            std::cout << "Result: " << result2 << std::endl;
        } else {
            std::cout << "Error: Operation failed" << std::endl;
        }
        
        // Test case 3: Valid division
        std::cout << "\\nTest 3: Division (100 / 4)" << std::endl;
        int result3 = 0;
        if (processOperation("divide", 100, 4, result3)) {
            std::cout << "Result: " << result3 << std::endl;
        } else {
            std::cout << "Error: Operation failed" << std::endl;
        }
        
        // Test case 4: Division by zero error
        std::cout << "\\nTest 4: Division by zero (10 / 0)" << std::endl;
        int result4 = 0;
        if (processOperation("divide", 10, 0, result4)) {
            std::cout << "Result: " << result4 << std::endl;
        } else {
            std::cout << "Error: Division by zero detected and prevented" << std::endl;
        }
        
        // Test case 5: Overflow detection
        std::cout << "\\nTest 5: Integer overflow (MAX_INT + 1)" << std::endl;
        int result5 = 0;
        if (processOperation("add", std::numeric_limits<int>::max(), 1, result5)) {
            std::cout << "Result: " << result5 << std::endl;
        } else {
            std::cout << "Error: Integer overflow detected and prevented" << std::endl;
        }
        
    } catch (const std::exception& e) {
        // Catch any unexpected exceptions
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        // Catch all other exceptions
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
