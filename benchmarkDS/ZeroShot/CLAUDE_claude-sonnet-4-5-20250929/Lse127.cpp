
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string sanitizeInput(const std::string& input) {
    std::string sanitized;
    for (char c : input) {
        if (!std::iscntrl(c)) {
            sanitized += c;
        }
    }
    return sanitized.substr(0, 100);
}

std::string escapeHtml(const std::string& input) {
    std::string escaped;
    for (char c : input) {
        switch (c) {
            case '&': escaped += "&amp;"; break;
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '"': escaped += "&quot;"; break;
            case '\\'': escaped += "&#x27;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

void httpHelloPage(std::istream& request, std::ostream& response) {
    std::string username;
    std::string line;
    
    // Read headers
    while (std::getline(request, line) && !line.empty() && line != "\\r") {
        // Skip headers
    }
    
    // Read body
    std::string body;
    std::getline(request, body);
    
    // Parse username
    size_t pos = body.find("username=");
    if (pos != std::string::npos) {
        size_t start = pos + 9;
        size_t end = body.find("&", start);
        if (end == std::string::npos) {
            end = body.length();
        }
        username = body.substr(start, end - start);
        username = urlDecode(username);
        username = sanitizeInput(username);
    }
    
    if (username.empty()) {
        username = "Guest";
    }
    
    // Write response
    response << "HTTP/1.1 200 OK\\r\\n";
    response << "Content-Type: text/html\\r\\n";
    response << "\\r\\n";
    response << "<html><body>\\r\\n";
    response << "<h1>Hello " << escapeHtml(username) << "</h1>\\r\\n";
    response << "</body></html>\\r\\n";
    response.flush();
}

int main() {
    std::cout << "Test cases for httpHelloPage:\\n";
    
    std::string testCases[] = {
        "username=John",
        "username=Alice%20Smith",
        "username=%3Cscript%3Ealert%28%29%3C%2Fscript%3E",
        "username=",
        "username=Bob&other=data"
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "\\nTest Case " << (i + 1) << ":\\n";
        std::string requestStr = "POST / HTTP/1.1\\r\\n\\r\\n" + testCases[i];
        std::istringstream request(requestStr);
        std::ostringstream response;
        
        httpHelloPage(request, response);
        std::cout << response.str() << std::endl;
    }
    
    return 0;
}
