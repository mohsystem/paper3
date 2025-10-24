
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <limits>
#include <stdexcept>

class Task44 {
private:
    static const int MAX_INPUT_LENGTH = 100;
    
public:
    static std::string sanitizeInput(const std::string& input) {
        if (input.empty()) {
            return "";
        }
        
        std::string result = input;
        
        // Trim whitespace
        result.erase(0, result.find_first_not_of(" \\t\\n\\r"));
        result.erase(result.find_last_not_of(" \\t\\n\\r") + 1);
        
        // Check length
        if (result.length() > MAX_INPUT_LENGTH) {
            throw std::invalid_argument("Input exceeds maximum length of " + std::to_string(MAX_INPUT_LENGTH));
        }
        
        // Remove potential SQL injection characters
        result.erase(std::remove_if(result.begin(), result.end(),
            [](char c) { return c == '\\'' || c == '"' || c == ';' || c == '\\\\'; }), result.end());\n        \n        // Remove potential XSS characters\n        result.erase(std::remove_if(result.begin(), result.end(),\n            [](char c) { return c == '<' || c == '>'; }), result.end());\n        \n        return result;\n    }\n    \n    static bool validateAlphanumeric(const std::string& input) {\n        if (input.empty()) {\n            return false;\n        }\n        std::regex pattern("^[a-zA-Z0-9\\\\s]+$");\n        return std::regex_match(input, pattern);\n    }\n    \n    static bool validateEmail(const std::string& input) {\n        if (input.empty() || input.length() > MAX_INPUT_LENGTH) {\n            return false;\n        }\n        std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");\n        return std::regex_match(input, pattern);\n    }\n    \n    static bool validateNumeric(const std::string& input) {\n        if (input.empty()) {\n            return false;\n        }\n        std::regex pattern("^\\\\d+$");\n        return std::regex_match(input, pattern);\n    }\n    \n    static int safeParseInt(const std::string& input, int defaultValue) {\n        try {\n            if (!validateNumeric(input)) {\n                return defaultValue;\n            }\n            long long value = std::stoll(input);\n            if (value > std::numeric_limits<int>::max() || value < std::numeric_limits<int>::min()) {\n                return defaultValue;\n            }\n            return static_cast<int>(value);\n        } catch (...) {\n            return defaultValue;\n        }\n    }\n    \n    static std::string processInput(const std::string& input, const std::string& inputType) {\n        if (input.empty() || inputType.empty()) {\n            return "Error: Null or empty input provided";\n        }\n        \n        try {\n            std::string sanitized = sanitizeInput(input);\n            std::string typeLower = inputType;\n            std::transform(typeLower.begin(), typeLower.end(), typeLower.begin(), ::tolower);\n            \n            if (typeLower == "alphanumeric") {\n                if (validateAlphanumeric(sanitized)) {\n                    return "Valid alphanumeric input: " + sanitized;\n                } else {\n                    return "Invalid alphanumeric input";\n                }\n            } else if (typeLower == "email") {\n                if (validateEmail(sanitized)) {\n                    return "Valid email: " + sanitized;\n                } else {\n                    return "Invalid email format";\n                }\n            } else if (typeLower == "numeric") {\n                if (validateNumeric(sanitized)) {\n                    int number = safeParseInt(sanitized, 0);\n                    return "Valid number: " + std::to_string(number) + ", Doubled: " + std::to_string(number * 2);\n                } else {\n                    return "Invalid numeric input";\n                }\n            } else {\n                return "Unknown input type";\n            }\n        } catch (const std::exception& e) {\n            return std::string("Error: ") + e.what();\n        }\n    }\n};\n\nint main() {\n    std::cout << "=== Secure Input Validation Test Cases ===\\n\\n";\n    \n    // Test Case 1: Valid alphanumeric input\n    std::cout << "Test 1 - Valid Alphanumeric:\\n";\n    std::cout << Task44::processInput("Hello World 123", "alphanumeric") << "\\n\\n";\n    \n    // Test Case 2: Valid email\n    std::cout << "Test 2 - Valid Email:\\n";\n    std::cout << Task44::processInput("user@example.com", "email") << "\\n\\n";\n    \n    // Test Case 3: Valid numeric input\n    std::cout << "Test 3 - Valid Numeric:\\n";\n    std::cout << Task44::processInput("42", "numeric") << "\\n\\n";\n    \n    // Test Case 4: SQL Injection attempt\n    std::cout << "Test 4 - SQL Injection Attempt:\\n";\n    std::cout << Task44::processInput("admin' OR '1'='1", "alphanumeric") << "\\n\\n";\n    \n    // Test Case 5: XSS attempt\n    std::cout << "Test 5 - XSS Attempt:\\n";\n    std::cout << Task44::processInput("<script>alert('XSS')</script>", "alphanumeric") << "\
\
";
    
    return 0;
}
