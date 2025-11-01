
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <regex>

const int MAX_USERNAME_LENGTH = 100;

class Lse128 {
public:
    static std::string http_hello_page(std::istream& request, std::ostream& response) {
        std::string username = extractUsername(request);
        
        if (username.empty()) {
            username = "Guest";
        }
        
        // Sanitize username
        username = sanitizeInput(username);
        
        // Construct HTTP response
        std::string message = "hello " + htmlEncode(username);
        
        response << "HTTP/1.1 200 OK\\r\\n";
        response << "Content-Type: text/html; charset=UTF-8\\r\\n";
        response << "Content-Length: " << message.length() << "\\r\\n";
        response << "\\r\\n";
        response << message;
        response.flush();
        
        return message;
    }
    
private:
    static std::string extractUsername(std::istream& request) {
        std::string line;
        while (std::getline(request, line)) {
            if (line.empty()) break;
            
            std::string lower_line = line;
            std::transform(lower_line.begin(), lower_line.end(), lower_line.begin(), ::tolower);
            
            if (lower_line.find("username:") == 0) {
                std::string username = line.substr(9);
                username.erase(0, username.find_first_not_of(" \\t\\r\\n"));
                username.erase(username.find_last_not_of(" \\t\\r\\n") + 1);
                return username;
            }
        }
        return "Guest";
    }
    
    static std::string sanitizeInput(const std::string& input) {
        if (input.empty()) return "";
        
        std::string result = input;
        
        // Limit length
        if (result.length() > MAX_USERNAME_LENGTH) {
            result = result.substr(0, MAX_USERNAME_LENGTH);
        }
        
        // Remove unsafe characters
        std::regex safe_pattern("[^a-zA-Z0-9_\\\\- ]");
        result = std::regex_replace(result, safe_pattern, "");
        
        // Trim
        result.erase(0, result.find_first_not_of(" \\t\\r\\n"));
        result.erase(result.find_last_not_of(" \\t\\r\\n") + 1);
        
        return result;
    }
    
    static std::string htmlEncode(const std::string& input) {
        std::string result;
        result.reserve(input.length() * 2);
        
        for (char c : input) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\\'': result += "&#x27;"; break;
                case '/': result += "&#x2F;"; break;
                default: result += c; break;
            }
        }
        
        return result;
    }
};

int main() {
    // Test case 1: Normal username
    {
        std::istringstream request("username: John");
        std::ostringstream response;
        std::string result = Lse128::http_hello_page(request, response);
        std::cout << "Test 1 - Input: username: John" << std::endl;
        std::cout << "Output: " << result << std::endl;
        std::cout << "---" << std::endl;
    }
    
    // Test case 2: Empty username
    {
        std::istringstream request("");
        std::ostringstream response;
        std::string result = Lse128::http_hello_page(request, response);
        std::cout << "Test 2 - Input: (empty)" << std::endl;
        std::cout << "Output: " << result << std::endl;
        std::cout << "---" << std::endl;
    }
    
    // Test case 3: XSS attempt
    {
        std::istringstream request("username: <script>alert('xss')</script>");
        std::ostringstream response;
        std::string result = Lse128::http_hello_page(request, response);
        std::cout << "Test 3 - Input: username: <script>alert('xss')</script>" << std::endl;
        std::cout << "Output: " << result << std::endl;
        std::cout << "---" << std::endl;
    }
    
    // Test case 4: Long username
    {
        std::string longName(150, 'a');
        std::istringstream request("username: " + longName);
        std::ostringstream response;
        std::string result = Lse128::http_hello_page(request, response);
        std::cout << "Test 4 - Input: username: " << longName.substr(0, 20) << "..." << std::endl;
        std::cout << "Output length: " << result.length() << std::endl;
        std::cout << "---" << std::endl;
    }
    
    // Test case 5: SQL injection attempt
    {
        std::istringstream request("username: admin' OR '1'='1");
        std::ostringstream response;
        std::string result = Lse128::http_hello_page(request, response);
        std::cout << "Test 5 - Input: username: admin' OR '1'='1" << std::endl;
        std::cout << "Output: " << result << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
