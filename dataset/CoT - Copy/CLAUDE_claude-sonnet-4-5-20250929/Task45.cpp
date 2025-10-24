
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <regex>
#include <limits>
#include <algorithm>
#include <cctype>

class Task45 {
private:
    static const int MAX_INPUT_LENGTH = 1000;
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }
    
    static std::string toLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    
public:
    // Secure calculator with input validation
    static double calculator(const std::string& operation, double num1, double num2) {
        if (operation.empty() || trim(operation).empty()) {
            throw std::invalid_argument("Operation cannot be null or empty");
        }
        
        if (std::isnan(num1) || std::isnan(num2) || 
            std::isinf(num1) || std::isinf(num2)) {
            throw std::invalid_argument("Invalid number input");
        }
        
        std::string op = toLower(trim(operation));
        
        if (op == "add") {
            return num1 + num2;
        } else if (op == "subtract") {
            return num1 - num2;
        } else if (op == "multiply") {
            return num1 * num2;
        } else if (op == "divide") {
            if (num2 == 0) {
                throw std::runtime_error("Division by zero is not allowed");
            }
            return num1 / num2;
        } else {
            throw std::invalid_argument("Invalid operation: " + operation);
        }
    }
    
    // Secure string validator with length and character checks
    static bool validateInput(const std::string& input) {
        if (input.length() > MAX_INPUT_LENGTH) {
            throw std::invalid_argument("Input exceeds maximum allowed length");
        }
        
        std::regex alphanumeric("^[a-zA-Z0-9\\\\s]+$");
        return std::regex_match(input, alphanumeric);
    }
    
    // Secure array processor with bounds checking
    static int processArray(const std::vector<int>& arr, int index) {
        if (arr.empty()) {
            throw std::invalid_argument("Array cannot be empty");
        }
        
        if (index < 0 || index >= static_cast<int>(arr.size())) {
            throw std::out_of_range("Index " + std::to_string(index) + 
                " is out of bounds for array length " + std::to_string(arr.size()));
        }
        
        return arr[index] * 2;
    }
    
    // Secure string parser with error handling
    static int parseSecureInteger(const std::string& input) {
        if (input.empty() || trim(input).empty()) {
            throw std::invalid_argument("Input cannot be null or empty");
        }
        
        std::string trimmed = trim(input);
        if (trimmed.length() > 10) {
            throw std::invalid_argument("Input string too long for integer parsing");
        }
        
        try {
            size_t pos;
            int result = std::stoi(trimmed, &pos);
            if (pos != trimmed.length()) {
                throw std::invalid_argument("Invalid integer format: " + trimmed);
            }
            return result;
        } catch (const std::invalid_argument&) {
            throw std::invalid_argument("Invalid integer format: " + trimmed);
        } catch (const std::out_of_range&) {
            throw std::out_of_range("Integer value out of range: " + trimmed);
        }
    }
};

int main() {
    std::cout << "=== Test Case 1: Calculator Operations ===" << std::endl;
    try {
        std::cout << "Add 10 + 5 = " << Task45::calculator("add", 10, 5) << std::endl;
        std::cout << "Divide 20 / 4 = " << Task45::calculator("divide", 20, 4) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\n=== Test Case 2: Division by Zero ===" << std::endl;
    try {
        Task45::calculator("divide", 10, 0);
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught expected error: " << e.what() << std::endl;
    }
    
    std::cout << "\\n=== Test Case 3: Input Validation ===" << std::endl;
    try {
        std::cout << "Valid input 'Hello123': " << Task45::validateInput("Hello123") << std::endl;
        std::cout << "Invalid input 'Hello@123': " << Task45::validateInput("Hello@123") << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\n=== Test Case 4: Array Processing ===" << std::endl;
    try {
        std::vector<int> testArray = {1, 2, 3, 4, 5};
        std::cout << "Process array[2] = " << Task45::processArray(testArray, 2) << std::endl;
        std::cout << "Process array[10] = " << Task45::processArray(testArray, 10) << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Caught expected error: " << e.what() << std::endl;
    }
    
    std::cout << "\\n=== Test Case 5: String Parsing ===" << std::endl;
    try {
        std::cout << "Parse '123' = " << Task45::parseSecureInteger("123") << std::endl;
        std::cout << "Parse 'abc' = " << Task45::parseSecureInteger("abc") << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Caught expected error: " << e.what() << std::endl;
    }
    
    return 0;
}
