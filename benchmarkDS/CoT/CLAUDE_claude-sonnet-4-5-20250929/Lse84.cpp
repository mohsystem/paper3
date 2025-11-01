
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <cstring>

// Note: This is a simplified version as C++ doesn't have built-in HTTP server\n// In production, use libraries like cpp-httplib or Boost.Beast\n// For YAML parsing, use yaml-cpp library with safe parsing options\n\nclass Lse84 {\nprivate:\n    static const int MAX_PAYLOAD_SIZE = 10000;\n\n    std::string extractPayload(const std::string& query) {\n        size_t pos = query.find("payload=");\n        if (pos == std::string::npos) return "";\n        \n        pos += 8; // length of "payload="\n        size_t end = query.find('&', pos);\n        if (end == std::string::npos) {\n            return query.substr(pos);\n        }\n        return query.substr(pos, end - pos);\n    }\n\n    std::string urlDecode(const std::string& str) {\n        std::string result;\n        for (size_t i = 0; i < str.length(); i++) {\n            if (str[i] == '%' && i + 2 < str.length()) {\n                int value;\n                std::istringstream iss(str.substr(i + 1, 2));\n                if (iss >> std::hex >> value) {\n                    result += static_cast<char>(value);\n                    i += 2;\n                }\n            } else if (str[i] == '+') {\n                result += ' ';\n            } else {\n                result += str[i];\n            }\n        }\n        return result;\n    }\n\n    std::string processPayload(const std::string& payload) {\n        if (payload.empty()) {\n            return "Error: payload parameter is empty";\n        }\n        \n        if (payload.length() > MAX_PAYLOAD_SIZE) {\n            return "Error: payload too large";\n        }\n\n        // Simplified YAML parsing check for "type: Create"\n        // In production, use yaml-cpp with safe parsing\n        if (payload.find("type:") != std::string::npos || \n            payload.find("type :") != std::string::npos) {\n            \n            size_t typePos = payload.find("type");\n            size_t colonPos = payload.find(':', typePos);
            if (colonPos != std::string::npos) {
                std::string value = payload.substr(colonPos + 1);
                size_t start = value.find_first_not_of(" \\t\\n\\r");
                if (start != std::string::npos) {
                    std::string typeValue = value.substr(start);
                    if (typeValue.find("Create") == 0) {
                        return "Error: Create type is not allowed";
                    }
                }
            }
        }
        
        return "Success: Payload processed";
    }

public:
    std::string handleRequest(const std::string& query) {
        if (query.find("payload=") == std::string::npos) {
            return "Error: payload parameter is missing";
        }
        
        std::string payload = extractPayload(query);
        payload = urlDecode(payload);
        
        return processPayload(payload);
    }

    static void runTests() {
        Lse84 server;
        
        std::cout << "\\nTest Cases:\\n";
        
        // Test 1: Missing payload
        std::cout << "Test 1: " << server.handleRequest("") << "\\n";
        
        // Test 2: Valid payload with type=Create
        std::cout << "Test 2: " << server.handleRequest("payload=type:%20Create") << "\\n";
        
        // Test 3: Valid payload with type=Update
        std::cout << "Test 3: " << server.handleRequest("payload=type:%20Update") << "\\n";
        
        // Test 4: Empty payload
        std::cout << "Test 4: " << server.handleRequest("payload=") << "\\n";
        
        // Test 5: Valid payload without type
        std::cout << "Test 5: " << server.handleRequest("payload=data:%20value") << "\\n";
    }
};

int main() {
    std::cout << "Server simulation on port 5000\\n";
    Lse84::runTests();
    return 0;
}
