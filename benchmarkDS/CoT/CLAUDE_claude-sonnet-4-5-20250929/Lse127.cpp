
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

const int MAX_USERNAME_LENGTH = 100;

std::string sanitizeInput(const std::string& input) {
    std::string sanitized;
    sanitized.reserve(input.length() * 2);
    
    for (char c : input) {
        switch (c) {
            case '<':
                sanitized += "&lt;";
                break;
            case '>':
                sanitized += "&gt;";
                break;
            case '&':
                sanitized += "&amp;";
                break;
            case '"':
                sanitized += "&quot;";
                break;
            case '\\'':
                sanitized += "&#x27;";
                break;
            case '/':
                sanitized += "&#x2F;";
                break;
            default:
                if (c >= 32 && c <= 126) {
                    sanitized += c;
                }
                break;
        }
    }
    return sanitized;
}

std::string httpHelloPage(std::istream& request, std::ostream& response) {
    std::string username;
    std::getline(request, username);
    
    // Remove leading/trailing whitespace
    username.erase(0, username.find_first_not_of(" \\t\\r\\n"));
    username.erase(username.find_last_not_of(" \\t\\r\\n") + 1);
    
    // Input validation and sanitization
    if (username.empty()) {
        username = "Guest";
    } else {
        if (username.length() > MAX_USERNAME_LENGTH) {
            username = username.substr(0, MAX_USERNAME_LENGTH);
        }
        username = sanitizeInput(username);
    }
    
    std::string responseMessage = "Hello " + username;
    response << responseMessage << std::endl;
    response.flush();
    
    return responseMessage;
}

int main() {
    std::string testCases[] = {
        "Alice",
        "Bob<script>alert('xss')</script>",
        "Charlie&Drop Table",
        "",
        std::string(150, 'A')
    };
    
    for (int i = 0; i < 5; i++) {
        std::istringstream request(testCases[i]);
        std::ostringstream response;
        
        std::string result = httpHelloPage(request, response);
        std::cout << "Test " << (i + 1) << ": " << result << std::endl;
    }
    
    return 0;
}
