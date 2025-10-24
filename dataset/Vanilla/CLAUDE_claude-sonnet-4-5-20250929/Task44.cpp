
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

class Task44 {
private:
    static const int MAX_INPUT_LENGTH = 100;
    static std::regex ALPHANUMERIC_PATTERN;
    static std::regex EMAIL_PATTERN;
    static std::regex NUMERIC_PATTERN;

public:
    static std::string validateAndSanitizeInput(const std::string& input) {
        if (input.empty()) {
            return "Error: Input cannot be null or empty";
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length of " + std::to_string(MAX_INPUT_LENGTH);
        }
        
        // Remove potentially dangerous characters
        std::string sanitized = input;
        std::string dangerousChars = "<>\\"';(){}[]\\\\";\n        sanitized.erase(\n            std::remove_if(sanitized.begin(), sanitized.end(),\n                [&dangerousChars](char c) {\n                    return dangerousChars.find(c) != std::string::npos;\n                }),\n            sanitized.end()\n        );\n        \n        // Trim whitespace\n        sanitized.erase(0, sanitized.find_first_not_of(" \\t\
\\r"));\n        sanitized.erase(sanitized.find_last_not_of(" \\t\
\\r") + 1);\n        \n        return sanitized;\n    }\n\n    static std::string validateAlphanumeric(const std::string& input) {\n        std::string sanitized = validateAndSanitizeInput(input);\n        if (sanitized.substr(0, 6) == "Error:") {\n            return sanitized;\n        }\n        \n        if (!std::regex_match(sanitized, ALPHANUMERIC_PATTERN)) {\n            return "Error: Input must contain only alphanumeric characters and spaces";\n        }\n        \n        return "Valid alphanumeric: " + sanitized;\n    }\n\n    static std::string validateEmail(const std::string& input) {\n        std::string sanitized = validateAndSanitizeInput(input);\n        if (sanitized.substr(0, 6) == "Error:") {\n            return sanitized;\n        }\n        \n        if (!std::regex_match(sanitized, EMAIL_PATTERN)) {\n            return "Error: Invalid email format";\n        }\n        \n        return "Valid email: " + sanitized;\n    }\n\n    static std::string validateNumeric(const std::string& input) {\n        std::string sanitized = validateAndSanitizeInput(input);\n        if (sanitized.substr(0, 6) == "Error:") {\n            return sanitized;\n        }\n        \n        if (!std::regex_match(sanitized, NUMERIC_PATTERN)) {\n            return "Error: Input must be a valid number";\n        }\n        \n        return "Valid number: " + sanitized;\n    }\n\n    static std::string processInput(const std::string& input, const std::string& type) {\n        if (type.empty()) {\n            return "Error: Validation type not specified";\n        }\n        \n        std::string lowerType = type;\n        std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);\n        \n        if (lowerType == "alphanumeric") {\n            return validateAlphanumeric(input);\n        } else if (lowerType == "email") {\n            return validateEmail(input);\n        } else if (lowerType == "numeric") {\n            return validateNumeric(input);\n        } else {\n            return "Error: Unknown validation type";\n        }\n    }\n};\n\nstd::regex Task44::ALPHANUMERIC_PATTERN = std::regex("^[a-zA-Z0-9\\\\s]+$");\nstd::regex Task44::EMAIL_PATTERN = std::regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");\nstd::regex Task44::NUMERIC_PATTERN = std::regex("^-?\\\\d+$");\n\nint main() {\n    std::cout << "=== Test Case 1: Valid Alphanumeric ===" << std::endl;\n    std::cout << Task44::processInput("Hello World 123", "alphanumeric") << std::endl;\n    \n    std::cout << "\
=== Test Case 2: Valid Email ===" << std::endl;\n    std::cout << Task44::processInput("user@example.com", "email") << std::endl;\n    \n    std::cout << "\
=== Test Case 3: Valid Numeric ===" << std::endl;\n    std::cout << Task44::processInput("12345", "numeric") << std::endl;\n    \n    std::cout << "\
=== Test Case 4: Invalid Input with Special Characters ===" << std::endl;\n    std::cout << Task44::processInput("<script>alert('xss')</script>", "alphanumeric") << std::endl;\n    \n    std::cout << "\
=== Test Case 5: Input Too Long ===" << std::endl;\n    std::string longInput(150, 'a');\n    std::cout << Task44::processInput(longInput, "alphanumeric") << std::endl;
    
    return 0;
}
