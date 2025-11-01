
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

class Lse83 {
public:
    static std::string processPayload(const std::map<std::string, std::string>* payload) {
        if (payload == nullptr) {
            return "Error: Invalid payload";
        }
        
        auto it = payload->find("name");
        if (it != payload->end()) {
            std::string name = sanitizeInput(it->second);
            return "Name: " + name;
        }
        
        return "Error: Field 'name' not found";
    }
    
private:
    static std::string sanitizeInput(const std::string& input) {
        std::string result;
        for (char c : input) {
            switch (c) {
                case '&':
                    result += "&amp;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '\\'':
                    result += "&#x27;";
                    break;
                case '/':
                    result += "&#x2F;";
                    break;
                default:
                    result += c;
            }
        }
        return result;
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::map<std::string, std::string> payload1;
    payload1["name"] = "John Doe";
    std::cout << Lse83::processPayload(&payload1) << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::map<std::string, std::string> payload2;
    payload2["email"] = "test@example.com";
    std::cout << Lse83::processPayload(&payload2) << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::map<std::string, std::string> payload3;
    payload3["name"] = "<script>alert('XSS')</script>";
    std::cout << Lse83::processPayload(&payload3) << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::map<std::string, std::string> payload4;
    payload4["name"] = "Alice & Bob";
    std::cout << Lse83::processPayload(&payload4) << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::cout << Lse83::processPayload(nullptr) << std::endl;
    
    return 0;
}
