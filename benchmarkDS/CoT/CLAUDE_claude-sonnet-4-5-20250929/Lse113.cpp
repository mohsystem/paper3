
#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <array>
#include <memory>

class Lse113 {
private:
    static bool isValidHostname(const std::string& url) {
        std::regex hostnamePattern(
            "^([a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?\\\\.)+[a-zA-Z]{2,}$"
        );
        std::regex ipPattern(
            "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );
        return std::regex_match(url, hostnamePattern) || std::regex_match(url, ipPattern);
    }
    
    static std::string sanitizeUrl(std::string url) {
        // Remove protocol
        size_t pos = url.find("://");
        if (pos != std::string::npos) {
            url = url.substr(pos + 3);
        }
        // Remove path
        pos = url.find('/');
        if (pos != std::string::npos) {
            url = url.substr(0, pos);
        }
        return url;
    }

public:
    static std::string ping(std::string url) {
        // Input validation
        if (url.empty()) {
            return "Error: Invalid input - URL cannot be empty";
        }
        
        url = sanitizeUrl(url);
        
        // Validate format
        if (!isValidHostname(url)) {
            return "Error: Invalid hostname or IP address format";
        }
        
        // Build safe command - use execl or similar in production
        // This is still safer than direct system() with user input
        std::string command;
        #ifdef _WIN32
            command = "ping -n 1 -w 5000 " + url + " > nul 2>&1";
        #else
            command = "ping -c 1 -W 5 " + url + " > /dev/null 2>&1";
        #endif
        
        // NOTE: In production, use proper exec functions with argument arrays
        // This is a simplified version - still validates input strictly
        int result = system(command.c_str());
        
        if (result == 0) {
            return "URL " + url + " is alive";
        } else {
            return "URL " + url + " is not alive";
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse113::ping("8.8.8.8") << std::endl;
    std::cout << "Test 2: " << Lse113::ping("google.com") << std::endl;
    std::cout << "Test 3: " << Lse113::ping("invalid..host") << std::endl;
    std::cout << "Test 4: " << Lse113::ping("; rm -rf /") << std::endl;
    std::cout << "Test 5: " << Lse113::ping("") << std::endl;
    
    return 0;
}
