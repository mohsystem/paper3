
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cstring>

std::string htmlEscape(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 1.5);
    
    for (char c : input) {
        switch (c) {
            case '&': output.append("&amp;"); break;
            case '<': output.append("&lt;"); break;
            case '>': output.append("&gt;"); break;
            case '"': output.append("&quot;"); break;
            case '\\'': output.append("&#x27;"); break;
            case '/': output.append("&#x2F;"); break;
            default: output.push_back(c);
        }
    }
    return output;
}

void http_hello_page(std::istream& requestBody, std::ostream& response) {
    try {
        // Read username from request body with size limit
        const size_t maxSize = 8192;
        std::string username;
        char buffer[1024];
        size_t totalBytes = 0;
        
        while (requestBody.read(buffer, sizeof(buffer)) || requestBody.gcount() > 0) {
            size_t bytesRead = requestBody.gcount();
            if (totalBytes + bytesRead > maxSize) {
                bytesRead = maxSize - totalBytes;
            }
            username.append(buffer, bytesRead);
            totalBytes += bytesRead;
            if (totalBytes >= maxSize) break;
        }
        
        // Trim whitespace
        username.erase(0, username.find_first_not_of(" \\t\\n\\r"));
        username.erase(username.find_last_not_of(" \\t\\n\\r") + 1);
        
        // Validate and sanitize username
        if (username.length() > 100) {
            username = username.substr(0, 100);
        }
        username = htmlEscape(username);
        
        // Construct HTML response
        std::ostringstream outputbuf;
        outputbuf << "HTTP/1.1 200 OK\\r\\n";
        outputbuf << "Content-Type: text/html; charset=UTF-8\\r\\n";
        outputbuf << "X-Content-Type-Options: nosniff\\r\\n";
        outputbuf << "X-Frame-Options: DENY\\r\\n";
        outputbuf << "\\r\\n";
        outputbuf << "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">";
        outputbuf << "<title>Hello Page</title></head><body>";
        outputbuf << "<h1>Hello, " << username << "!</h1>";
        outputbuf << "</body></html>";
        
        // Write response
        response << outputbuf.str();
        response.flush();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test cases
    std::string testUsernames[] = {
        "Alice",
        "Bob<script>alert('xss')</script>",
        "Charlie&Sons",
        "Diana\\"O'Neil\\"",
        ""
    };
    
    for (const auto& testUser : testUsernames) {
        std::cout << "Testing with username: " << testUser << std::endl;
        std::istringstream input(testUser);
        std::ostringstream output;
        http_hello_page(input, output);
        std::cout << output.str() << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
