
#include <iostream>
#include <string>
#include <regex>
#include <array>
#include <memory>
#include <stdexcept>
#include <cstring>

class Lse112 {
private:
    static bool isValidUrl(const std::string& url) {
        if (url.empty() || url.length() > 253) {
            return false;
        }
        
        // Only allow alphanumeric, dots, hyphens, and colons
        std::regex urlPattern("^[a-zA-Z0-9.-:]+$");
        return std::regex_match(url, urlPattern);
    }
    
public:
    static std::string ping(const std::string& url) {
        try {
            // Validate URL to prevent command injection
            if (!isValidUrl(url)) {
                return "Invalid URL format. Only alphanumeric characters, dots, hyphens, and colons are allowed.";
            }
            
            // Build command safely
            std::string command;
            #ifdef _WIN32
                command = "ping -n 4 " + url + " 2>&1";
            #else
                command = "ping -c 4 " + url + " 2>&1";
            #endif
            
            // Execute command
            std::array<char, 128> buffer;
            std::string result = "Ping results for " + url + ":\\n";
            
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
            if (!pipe) {
                return "Error executing ping command";
            }
            
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            
            return result;
            
        } catch (const std::exception& e) {
            return std::string("Error executing ping: ") + e.what();
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: Valid domain" << std::endl;
    std::cout << Lse112::ping("google.com") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test 2: Valid IP address" << std::endl;
    std::cout << Lse112::ping("8.8.8.8") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test 3: Localhost" << std::endl;
    std::cout << Lse112::ping("localhost") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test 4: Invalid URL with command injection attempt" << std::endl;
    std::cout << Lse112::ping("google.com; ls") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test 5: Invalid URL with special characters" << std::endl;
    std::cout << Lse112::ping("google.com && whoami") << std::endl;
    
    return 0;
}
