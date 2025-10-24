
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <cstring>
#include <algorithm>

// Note: C++ does not have a standard XML-RPC library.
// This is a simplified demonstration of the concept.
// For production use, consider libraries like xmlrpc-c or XML-RPC++

class Task111 {
private:
    // Simple XML response builder
    static std::string buildXMLResponse(const std::string& value, const std::string& type) {
        std::ostringstream oss;
        oss << "<?xml version=\\"1.0\\"?>\\n";
        oss << "<methodResponse>\\n";
        oss << "  <params>\\n";
        oss << "    <param>\\n";
        oss << "      <value><" << type << ">" << value << "</" << type << "></value>\\n";
        oss << "    </param>\\n";
        oss << "  </params>\\n";
        oss << "</methodResponse>\\n";
        return oss.str();
    }
    
    // Sanitize input to prevent injection attacks
    static std::string sanitizeInput(const std::string& input) {
        std::string sanitized = input;
        // Remove potentially dangerous characters
        sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
            [](char c) { return c == '<' || c == '>' || c == '&' || c == '"' || c == '\\''; }),
            sanitized.end());
        return sanitized;
    }

public:
    // Mathematical operations
    static int add(int a, int b) {
        return a + b;
    }
    
    static int subtract(int a, int b) {
        return a - b;
    }
    
    static int multiply(int a, int b) {
        return a * b;
    }
    
    static double divide(int a, int b) {
        if (b == 0) {
            std::cerr << "Error: Division by zero" << std::endl;
            return 0.0;
        }
        return static_cast<double>(a) / b;
    }
    
    static std::string echo(const std::string& message) {
        if (message.empty()) {
            return "Empty message";
        }
        // Sanitize input
        std::string sanitized = sanitizeInput(message);
        return "Echo: " + sanitized;
    }
    
    // Simulated RPC handler
    static std::string handleRequest(const std::string& method, const std::string& params) {
        std::cout << "Handling method: " << method << std::endl;
        
        if (method == "Math.add") {
            int result = add(10, 5);
            return buildXMLResponse(std::to_string(result), "int");
        }
        else if (method == "Math.subtract") {
            int result = subtract(20, 8);
            return buildXMLResponse(std::to_string(result), "int");
        }
        else if (method == "Math.multiply") {
            int result = multiply(7, 6);
            return buildXMLResponse(std::to_string(result), "int");
        }
        else if (method == "Math.divide") {
            double result = divide(100, 4);
            return buildXMLResponse(std::to_string(result), "double");
        }
        else if (method == "Math.echo") {
            std::string result = echo(params);
            return buildXMLResponse(result, "string");
        }
        
        return buildXMLResponse("Unknown method", "string");
    }
    
    static void runTests() {
        std::cout << "Starting XML-RPC Server Tests...\\n" << std::endl;
        
        std::cout << "=== Test Case 1: Addition ===" << std::endl;
        std::cout << "add(10, 5) = " << add(10, 5) << std::endl;
        
        std::cout << "\\n=== Test Case 2: Subtraction ===" << std::endl;
        std::cout << "subtract(20, 8) = " << subtract(20, 8) << std::endl;
        
        std::cout << "\\n=== Test Case 3: Multiplication ===" << std::endl;
        std::cout << "multiply(7, 6) = " << multiply(7, 6) << std::endl;
        
        std::cout << "\\n=== Test Case 4: Division ===" << std::endl;
        std::cout << "divide(100, 4) = " << divide(100, 4) << std::endl;
        
        std::cout << "\\n=== Test Case 5: Echo with sanitization ===" << std::endl;
        std::cout << "echo('Hello World') = " << echo("Hello World") << std::endl;
        std::cout << "echo('<script>alert(1)</script>') = " << echo("<script>alert(1)</script>") << std::endl;
        
        std::cout << "\\n=== Simulated XML-RPC Responses ===" << std::endl;
        std::cout << handleRequest("Math.add", "") << std::endl;
    }
};

int main() {
    Task111::runTests();
    return 0;
}
