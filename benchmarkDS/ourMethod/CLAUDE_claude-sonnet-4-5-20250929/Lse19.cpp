
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>

/**
 * Adds 1000 to the provided integer value.
 * 
 * @param value The input integer value
 * @return The input value plus 1000
 * @throws std::overflow_error if addition would cause overflow
 */
int addThousand(int value) {
    // Check for integer overflow before performing addition
    // If value > INT_MAX - 1000, overflow would occur
    if (value > std::numeric_limits<int>::max() - 1000) {
        throw std::overflow_error("Integer overflow: result would exceed maximum value");
    }
    // Check for integer underflow
    if (value < std::numeric_limits<int>::min() + 1000) {
        throw std::overflow_error("Integer underflow: result would be below minimum value");
    }
    return value + 1000;
}

/**
 * Safely parses a string to integer with validation.
 * 
 * @param str The input string
 * @return Parsed integer value
 * @throws std::invalid_argument if parsing fails or input is invalid
 */
int safeParseInt(const char* str) {
    // Validate input is not NULL
    if (str == nullptr) {
        throw std::invalid_argument("Input is null");
    }
    
    // Validate input length to prevent excessive processing
    size_t len = std::strlen(str);
    if (len == 0 || len > 15) {
        throw std::invalid_argument("Invalid input length");
    }
    
    // Use strtol for safer parsing than atoi
    char* endptr = nullptr;
    errno = 0;
    long result = std::strtol(str, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || result > std::numeric_limits<int>::max() || 
        result < std::numeric_limits<int>::min()) {
        throw std::overflow_error("Value out of integer range");
    }
    
    // Check if entire string was converted
    if (endptr == str || *endptr != '\\0') {
        throw std::invalid_argument("Invalid input format");
    }
    
    return static_cast<int>(result);
}

int main(int argc, char* argv[]) {
    // Test cases
    int testCases[] = {0, 100, -500, 2147482647, -2147482648};
    
    for (int i = 0; i < 5; i++) {
        try {
            int result = addThousand(testCases[i]);
            std::cout << "Input: " << testCases[i] << ", Result: " << result << std::endl;
        } catch (const std::exception& e) {
            // Generic error message - don't leak internal details\n            std::cerr << "Error processing value " << testCases[i] << ": " << e.what() << std::endl;\n        }\n    }\n    \n    // If command line argument provided, process it\n    if (argc > 1) {\n        try {\n            // Validate and parse input\n            int inputValue = safeParseInt(argv[1]);\n            int result = addThousand(inputValue);\n            std::cout << "Command line input: " << inputValue << ", Result: " << result << std::endl;\n        } catch (const std::exception& e) {\n            // Generic error - don't expose internal details
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }
    
    return 0;
}
