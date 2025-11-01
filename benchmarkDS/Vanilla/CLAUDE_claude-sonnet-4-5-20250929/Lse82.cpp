
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <cstring>

// Simple JSON parser for name extraction
std::string getNameFromPayload(const std::string& payload) {
    size_t namePos = payload.find("\\"name\\"");
    if (namePos == std::string::npos) {
        return "Name not found";
    }
    
    size_t colonPos = payload.find(":", namePos);
    if (colonPos == std::string::npos) {
        return "Invalid payload";
    }
    
    size_t startQuote = payload.find("\\"", colonPos);
    if (startQuote == std::string::npos) {
        return "Invalid payload";
    }
    
    size_t endQuote = payload.find("\\"", startQuote + 1);
    if (endQuote == std::string::npos) {
        return "Invalid payload";
    }
    
    return payload.substr(startQuote + 1, endQuote - startQuote - 1);
}

// Note: Full web server implementation in C++ requires external libraries
// This demonstrates the core functionality
int main() {
    std::cout << "Test Cases:" << std::endl;
    std::cout << "1. " << getNameFromPayload("{\\"name\\":\\"John\\"}") << std::endl;
    std::cout << "2. " << getNameFromPayload("{\\"name\\":\\"Alice\\",\\"age\\":30}") << std::endl;
    std::cout << "3. " << getNameFromPayload("{\\"age\\":25}") << std::endl;
    std::cout << "4. " << getNameFromPayload("invalid json") << std::endl;
    std::cout << "5. " << getNameFromPayload("{\\"name\\":\\"\\"}") << std::endl;
    
    std::cout << "\\nNote: Full web server requires external libraries like Boost.Beast or cpp-httplib" << std::endl;
    
    return 0;
}
