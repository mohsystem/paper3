
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <algorithm>

std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream(str.substr(i + 1, 2)) >> std::hex >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

void httpHelloPage(std::istream& request, std::ostream& response) {
    std::string username = "";
    std::string line;
    
    // Read the request to extract username
    while (std::getline(request, line) && !line.empty()) {
        // Remove carriage return if present
        if (!line.empty() && line.back() == '\\r') {
            line.pop_back();
        }
        
        // Look for username parameter in the request line
        if (line.find("username=") != std::string::npos) {
            std::regex pattern("username=([^&\\\\s]+)");
            std::smatch match;
            if (std::regex_search(line, match, pattern)) {
                username = urlDecode(match[1].str());
            }
        }
    }
    
    // Construct and write the response
    response << "HTTP/1.1 200 OK\\n";
    response << "Content-Type: text/html\\n";
    response << "\\n";
    response << "<html><body>\\n";
    response << "<h1>hello " << username << "</h1>\\n";
    response << "</body></html>\\n";
    response.flush();
}

void testCase(const std::string& requestStr, const std::string& expectedUser) {
    std::cout << "Request: ";
    std::string displayReq = requestStr;
    std::replace(displayReq.begin(), displayReq.end(), '\\n', ' ');
    std::cout << displayReq << std::endl;
    
    std::istringstream request(requestStr);
    std::ostringstream response;
    
    httpHelloPage(request, response);
    
    std::cout << "Response:\\n" << response.str();
    std::cout << "---" << std::endl;
}

int main() {
    std::cout << "Testing HTTP Hello Page Function:\\n" << std::endl;
    
    // Test case 1
    testCase("GET /hello?username=John HTTP/1.1\\n", "John");
    
    // Test case 2
    testCase("GET /hello?username=Alice HTTP/1.1\\n", "Alice");
    
    // Test case 3
    testCase("GET /hello?username=Bob123 HTTP/1.1\\n", "Bob123");
    
    // Test case 4
    testCase("POST /hello HTTP/1.1\\nusername=Charlie\\n", "Charlie");
    
    // Test case 5
    testCase("GET /hello?username=Test%20User HTTP/1.1\\n", "Test User");
    
    return 0;
}
