
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

class Lse126 {
public:
    static std::string sanitizeInput(const std::string& input) {
        std::string result = input;
        std::string::size_type pos = 0;
        
        // Replace & with &amp;
        while ((pos = result.find("&", pos)) != std::string::npos) {
            result.replace(pos, 1, "&amp;");
            pos += 5;
        }
        
        // Replace < with &lt;
        pos = 0;
        while ((pos = result.find("<", pos)) != std::string::npos) {
            result.replace(pos, 1, "&lt;");
            pos += 4;
        }
        
        // Replace > with &gt;
        pos = 0;
        while ((pos = result.find(">", pos)) != std::string::npos) {
            result.replace(pos, 1, "&gt;");
            pos += 4;
        }
        
        // Replace " with &quot;\n        pos = 0;\n        while ((pos = result.find("\\"", pos)) != std::string::npos) {
            result.replace(pos, 1, "&quot;");
            pos += 6;
        }
        
        return result;
    }
    
    static std::string httpHelloPage(const std::string& username) {
        std::string sanitizedUsername = sanitizeInput(username);
        
        std::ostringstream outputbuf;
        outputbuf << "<!DOCTYPE html>\\n"
                  << "<html>\\n"
                  << "<head>\\n"
                  << "    <title>Hello Page</title>\\n"
                  << "</head>\\n"
                  << "<body>\\n"
                  << "    <h1>Hello, " << sanitizedUsername << "!</h1>\\n"
                  << "</body>\\n"
                  << "</html>";
        
        return outputbuf.str();
    }
    
    static std::string createHttpResponse(const std::string& body) {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\\r\\n";
        response << "Content-Type: text/html; charset=utf-8\\r\\n";
        response << "Content-Length: " << body.length() << "\\r\\n";
        response << "Connection: close\\r\\n";
        response << "\\r\\n";
        response << body;
        return response.str();
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1: Normal username" << std::endl;
    std::string response1 = Lse126::httpHelloPage("John");
    std::cout << response1 << std::endl << std::endl;
    
    std::cout << "Test Case 2: Username with special characters" << std::endl;
    std::string response2 = Lse126::httpHelloPage("Alice & Bob");
    std::cout << response2 << std::endl << std::endl;
    
    std::cout << "Test Case 3: Username with HTML tags (XSS attempt)" << std::endl;
    std::string response3 = Lse126::httpHelloPage("<script>alert('XSS')</script>");
    std::cout << response3 << std::endl << std::endl;
    
    std::cout << "Test Case 4: Username with quotes" << std::endl;
    std::string response4 = Lse126::httpHelloPage("O'Brien");
    std::cout << response4 << std::endl << std::endl;
    
    std::cout << "Test Case 5: Empty username" << std::endl;
    std::string response5 = Lse126::httpHelloPage("");
    std::cout << response5 << std::endl;
    
    return 0;
}
