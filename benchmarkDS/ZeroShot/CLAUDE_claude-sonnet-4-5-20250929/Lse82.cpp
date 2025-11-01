
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

// Note: C++ doesn't have built-in web server or JSON parsing like Python/Java\n// This is a simplified version showing the core logic\n// For production, you would need libraries like Crow/Beast for web server and nlohmann/json for JSON\n\nstd::string urlDecode(const std::string& str) {\n    std::string result;\n    char ch;\n    int i, ii;\n    for (i = 0; i < str.length(); i++) {\n        if (str[i] == '%') {\n            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);\n            ch = static_cast<char>(ii);\n            result += ch;\n            i = i + 2;\n        } else if (str[i] == '+') {\n            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\\"" + key + "\\":\\"";
    size_t pos = json.find(searchKey);
    
    if (pos != std::string::npos) {
        pos += searchKey.length();
        size_t endPos = json.find("\\"", pos);
        if (endPos != std::string::npos) {
            return json.substr(pos, endPos - pos);
        }
    }
    return "Name not found";
}

std::string processPayload(const std::string& payload) {
    try {
        std::string decoded = urlDecode(payload);
        return extractJsonValue(decoded, "name");
    } catch (...) {
        return "Error processing payload";
    }
}

int main() {
    // Test cases
    std::cout << "Test Cases:" << std::endl;
    std::cout << "1. " << processPayload("%7B%22name%22%3A%22John%22%7D") << std::endl;
    std::cout << "2. " << processPayload("%7B%22name%22%3A%22Alice%22%2C%22age%22%3A30%7D") << std::endl;
    std::cout << "3. " << processPayload("%7B%22age%22%3A25%7D") << std::endl;
    std::cout << "4. " << processPayload("%7B%22name%22%3A%22Bob%20Smith%22%7D") << std::endl;
    std::cout << "5. " << processPayload("invalid") << std::endl;
    
    std::cout << "\\nNote: Full web server implementation requires external libraries" << std::endl;
    
    return 0;
}
