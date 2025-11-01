
#include <iostream>
#include <cstring>
#include <cerrno>
#include <climits>
#include <cstdlib>

// Process input value by adding 1000 and printing result
// Returns 0 on success, -1 on error
int processValue(const char* input) {
    // Input validation: check if input is NULL or empty
    if (input == nullptr || input[0] == '\\0') {
        std::cerr << "Error: Input cannot be NULL or empty" << std::endl;
        return -1;
    }
    
    // Validate input contains only digits, optional sign, and whitespace
    const char* ptr = input;
    while (*ptr == ' ' || *ptr == '\\t') ptr++; // Skip leading whitespace
    
    if (*ptr == '-' || *ptr == '+') ptr++; // Skip sign
    
    bool hasDigit = false;
    while (*ptr != '\\0') {
        if (*ptr >= '0' && *ptr <= '9') {
            hasDigit = true;
            ptr++;
        } else if (*ptr == ' ' || *ptr == '\\t') {
            // Allow trailing whitespace
            while (*ptr == ' ' || *ptr == '\\t') ptr++;
            break;
        } else {
            std::cerr << "Error: Invalid number format" << std::endl;
            return -1;
        }
    }
    
    if (!hasDigit) {
        std::cerr << "Error: Invalid number format" << std::endl;
        return -1;
    }
    
    // Parse input as long with validation
    char* endptr = nullptr;
    errno = 0;
    long value = std::strtol(input, &endptr, 10);
    
    // Check for parsing errors
    if (errno == ERANGE || (errno != 0 && value == 0)) {
        std::cerr << "Error: Number out of range" << std::endl;
        return -1;
    }
    
    if (endptr == input) {
        std::cerr << "Error: Invalid number format" << std::endl;
        return -1;
    }
    
    // Check for integer overflow before addition
    if (value > LONG_MAX - 1000) {
        std::cerr << "Error: Value would overflow" << std::endl;
        return -1;
    }
    
    if (value < LONG_MIN + 1000) {
        std::cerr << "Error: Value would underflow" << std::endl;
        return -1;
    }
    
    // Safe addition after overflow check
    long result = value + 1000;
    
    // Print result to console using safe output method
    std::cout << result << std::endl;
    
    return 0;
}

int main() {
    // Test cases with input validation
    std::cout << "Test 1: Valid positive number" << std::endl;
    processValue("500");
    
    std::cout << "\\nTest 2: Valid negative number" << std::endl;
    processValue("-200");
    
    std::cout << "\\nTest 3: Zero" << std::endl;
    processValue("0");
    
    std::cout << "\\nTest 4: Large number" << std::endl;
    processValue("999999");
    
    std::cout << "\\nTest 5: Invalid input" << std::endl;
    processValue("not_a_number");
    
    return 0;
}
