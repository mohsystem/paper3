
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

std::string urlDecode(const std::string& str) {
    std::string result;
    char ch;
    int i, j;
    for (i = 0; i < str.length(); i++) {
        if (str[i] == '+') {
            result += ' ';
        } else if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &j);
            ch = static_cast<char>(j);
            result += ch;
            i = i + 2;
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string sanitizeInput(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\\'': output += "&#x27;"; break;
            case '/': output += "&#x2F;"; break;
            default: output += c; break;
        }
    }
    return output;
}

void httpHelloPage(std::istream& request, std::ostream& response) {
    try {
        std::string username = "";
        std::string line;
        
        // Read the request
        while (std::getline(request, line) && !line.empty()) {
            // Remove carriage return if present
            if (!line.empty() && line.back() == '\\r') {
                line.pop_back();
            }
            
            // Look for username parameter
            size_t pos = line.find("username=");
            if (pos != std::string::npos) {
                size_t startIndex = pos + 9;
                size_t endIndex = line.find("&", startIndex);
                if (endIndex == std::string::npos) {
                    endIndex = line.find(" ", startIndex);
                }
                if (endIndex == std::string::npos) {
                    endIndex = line.length();
                }
                
                username = line.substr(startIndex, endIndex - startIndex);
                username = urlDecode(username);
                break;
            }
        }
        
        // Sanitize username to prevent XSS attacks
        username = sanitizeInput(username);
        
        // Construct and send response
        response << "HTTP/1.1 200 OK\\n";
        response << "Content-Type: text/html; charset=UTF-8\\n";
        response << "\\n";
        response << "<html><body>\\n";
        response << "<h1>hello " << username << "</h1>\\n";
        response << "</body></html>\\n";
        response.flush();
        
    } catch (const std::exception& e) {
        response << "HTTP/1.1 500 Internal Server Error\\n";
        response << "\\n";
        response.flush();
    }
}

void testHttpHelloPage(const std::string& requestLine) {
    std::istringstream request(requestLine + "\\n\\n");
    std::ostringstream response;
    
    httpHelloPage(request, response);
    
    std::cout << "Test: " << requestLine << std::endl;
    std::cout << response.str() << std::endl;
    std::cout << "---" << std::endl;
}

int main() {
    // Test case 1: Normal username
    testHttpHelloPage("GET /?username=John HTTP/1.1");
    
    // Test case 2: Username with spaces
    testHttpHelloPage("GET /?username=John+Doe HTTP/1.1");
    
    // Test case 3: Empty username
    testHttpHelloPage("GET /?username= HTTP/1.1");
    
    // Test case 4: Username with special characters (XSS attempt)
    testHttpHelloPage("GET /?username=<script>alert('xss')</script> HTTP/1.1");
    
    // Test case 5: Username with encoded characters
    testHttpHelloPage("GET /?username=Alice%20Smith HTTP/1.1");
    
    return 0;
}
