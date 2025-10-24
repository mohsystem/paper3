
#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>
#include <cstdlib>
#include <cerrno>
#include <cctype>

class Task71 {
public:
    /**
     * Safely converts a string to an integer with proper error handling
     * @param input The string to convert
     * @param result Pointer to store the converted integer
     * @return true if conversion successful, false otherwise
     */
    static bool convertToInteger(const std::string& input, int& result) {
        // Input validation
        if (input.empty()) {
            std::cerr << "Error: Input is empty" << std::endl;
            return false;
        }
        
        // Trim whitespace
        size_t start = input.find_first_not_of(" \\t\\n\\r");
        size_t end = input.find_last_not_of(" \\t\\n\\r");
        
        if (start == std::string::npos) {
            std::cerr << "Error: Input contains only whitespace" << std::endl;
            return false;
        }
        
        std::string trimmed = input.substr(start, end - start + 1);
        
        // Validate format (optional sign followed by digits)
        size_t idx = 0;
        if (trimmed[idx] == '+' || trimmed[idx] == '-') {
            idx++;
        }
        
        if (idx >= trimmed.length() || !std::isdigit(trimmed[idx])) {
            std::cerr << "Error: Invalid integer format: '" << input << "'" << std::endl;
            return false;
        }
        
        // Use strtol for safe conversion
        char* endptr;
        errno = 0;
        long value = std::strtol(trimmed.c_str(), &endptr, 10);
        
        // Check for conversion errors
        if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
            std::cerr << "Error: Value out of integer range: " << input << std::endl;
            return false;
        }
        
        if (*endptr != '\\0') {
            std::cerr << "Error: Invalid characters in input: '" << input << "'" << std::endl;
            return false;
        }
        
        result = static_cast<int>(value);
        return true;
    }
};

int main(int argc, char* argv[]) {
    // Security: Validate command line arguments exist
    if (argc < 2) {
        std::cerr << "Error: No command line arguments provided" << std::endl;
        std::cout << "Usage: " << argv[0] << " <integer_string>" << std::endl;
        return 1;
    }
    
    std::cout << "=== Command Line Argument Conversion ===" << std::endl;
    for (int i = 1; i < argc; i++) {
        std::cout << "\\nArgument " << i << ": " << argv[i] << std::endl;
        int result;
        if (Task71::convertToInteger(argv[i], result)) {
            std::cout << "Converted successfully: " << result << std::endl;
        }
    }
    
    // Test cases
    std::cout << "\\n=== Running Test Cases ===" << std::endl;
    std::string testCases[] = {
        "42",                    // Valid positive integer
        "-123",                  // Valid negative integer
        "0",                     // Zero
        "2147483647",           // MAX_VALUE
        "invalid123"            // Invalid format
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "\\nTest Case " << (i + 1) << ": \\"" << testCases[i] << "\\"" << std::endl;
        int result;
        if (Task71::convertToInteger(testCases[i], result)) {
            std::cout << "Result: " << result << std::endl;
        }
    }
    
    return 0;
}
