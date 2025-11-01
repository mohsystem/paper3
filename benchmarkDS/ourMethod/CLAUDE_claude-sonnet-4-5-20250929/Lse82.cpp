
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cctype>

// Security: Maximum payload size to prevent DoS
const size_t MAX_PAYLOAD_SIZE = 10240; // 10KB limit

// Simple JSON parser for extracting name field
// Security: This is a minimal parser that avoids eval or dynamic code execution
class SimpleJsonParser {
public:
    // Security: Parse JSON safely without executing code
    static std::map<std::string, std::string> parse(const std::string& json) {
        std::map<std::string, std::string> result;
        
        // Security: Validate input size
        if (json.empty() || json.length() > MAX_PAYLOAD_SIZE) {
            return result;
        }
        
        // Security: Find the bounds of the JSON object
        size_t start = json.find('{');
        size_t end = json.rfind('}');
        
        if (start == std::string::npos || end == std::string::npos || start >= end) {
            return result;
        }
        
        // Extract content between braces
        std::string content = json.substr(start + 1, end - start - 1);
        
        // Security: Parse key-value pairs with bounds checking
        size_t pos = 0;
        while (pos < content.length()) {
            // Skip whitespace
            while (pos < content.length() && std::isspace(content[pos])) {
                pos++;
            }
            
            if (pos >= content.length()) break;
            
            // Find key
            if (content[pos] != '"') break;
            pos++;
            size_t keyStart = pos;
            while (pos < content.length() && content[pos] != '"') {
                if (content[pos] == '\\\\') pos++; // Skip escaped chars\n                pos++;\n            }\n            if (pos >= content.length()) break;\n            \n            std::string key = content.substr(keyStart, pos - keyStart);\n            pos++;\n            \n            // Skip to colon\n            while (pos < content.length() && content[pos] != ':') {\n                pos++;\n            }\n            if (pos >= content.length()) break;\n            pos++;\n            \n            // Skip whitespace\n            while (pos < content.length() && std::isspace(content[pos])) {\n                pos++;\n            }\n            \n            if (pos >= content.length()) break;\n            \n            // Find value\n            std::string value;\n            if (content[pos] == '"') {\n                pos++;\n                size_t valueStart = pos;\n                while (pos < content.length() && content[pos] != '"') {\n                    if (content[pos] == '\\\\') pos++; // Skip escaped chars\n                    pos++;\n                }\n                if (pos < content.length()) {\n                    value = content.substr(valueStart, pos - valueStart);\n                    pos++;\n                }\n            }\n            \n            if (!key.empty()) {\n                result[key] = value;\n            }\n            \n            // Skip to next comma or end\n            while (pos < content.length() && content[pos] != ',' && content[pos] != '}') {\n                pos++;\n            }\n            if (pos < content.length() && content[pos] == ',') {\n                pos++;\n            }\n        }\n        \n        return result;\n    }\n    \n    // Security: Escape special characters for JSON output\n    static std::string escapeJson(const std::string& input) {\n        std::string output;\n        output.reserve(input.length() * 2); // Reserve space to avoid reallocations\n        \n        for (size_t i = 0; i < input.length(); i++) {\n            char c = input[i];\n            switch (c) {\n                case '\\\\': output += "\\\\\\\\"; break;\n                case '"':  output += "\\\\\\""; break;
                case '\\n': output += "\\\\n"; break;
                case '\\r': output += "\\\\r"; break;
                case '\\t': output += "\\\\t"; break;
                default:   output += c; break;
            }
        }
        return output;
    }
};

// Security: Process payload with input validation and safe parsing
std::string processPayload(const std::string& payload) {
    // Security: Validate input is not empty and within size limits
    if (payload.empty()) {
        return "{\\"error\\":\\"Invalid input\\"}";
    }
    
    if (payload.length() > MAX_PAYLOAD_SIZE) {
        return "{\\"error\\":\\"Payload too large\\"}";
    }
    
    // Security: Parse JSON safely without code execution
    std::map<std::string, std::string> dict = SimpleJsonParser::parse(payload);
    
    // Security: Check if 'name' key exists
    if (dict.find("name") == dict.end()) {
        return "{\\"error\\":\\"Missing name field\\"}";
    }
    
    // Security: Get name value and sanitize for output
    std::string name = dict["name"];
    std::string escapedName = SimpleJsonParser::escapeJson(name);
    
    return "{\\"name\\":\\"" + escapedName + "\\"}";
}

int main() {
    std::cout << "Web API simulation (C++ version)" << std::endl;
    std::cout << "Note: C++ doesn't have built-in HTTP server." << std::endl;
    std::cout << "This demonstrates the payload processing logic.\\n" << std::endl;
    
    // Test cases
    std::cout << "Test cases:" << std::endl;
    std::cout << "1. Valid JSON: " << processPayload("{\\"name\\":\\"John\\"}") << std::endl;
    std::cout << "2. Valid JSON with special chars: " << processPayload("{\\"name\\":\\"O'Brien\\"}") << std::endl;
    std::cout << "3. Missing name field: " << processPayload("{\\"age\\":30}") << std::endl;
    std::cout << "4. Invalid JSON: " << processPayload("{invalid}") << std::endl;
    std::cout << "5. Empty payload: " << processPayload("") << std::endl;
    
    return 0;
}
