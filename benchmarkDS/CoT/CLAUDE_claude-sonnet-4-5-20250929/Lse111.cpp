
// Note: C++ doesn't have native web framework decorators like Flask
// This example uses C++ with libcurl-like structure
// Compile: g++ -o Lse111 Lse111.cpp -std=c++17

#include <iostream>
#include <string>
#include <regex>
#include <array>
#include <memory>
#include <cstdio>
#include <stdexcept>

class Lse111 {
private:
    std::regex domain_pattern;
    std::regex ip_pattern;
    
    bool validateInput(const std::string& url) {
        if (url.empty() || url.length() > 255) {
            return false;
        }
        
        return std::regex_match(url, domain_pattern) || 
               std::regex_match(url, ip_pattern);
    }
    
    std::string executeCommand(const std::string& cmd) {
        std::array<char, 128> buffer;
        std::string result;
        
        #ifdef _WIN32
            std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
        #else
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        #endif
        
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result;
    }
    
public:
    Lse111() {
        domain_pattern = std::regex(
            "^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)+[a-zA-Z]{2,}$"
        );
        
        ip_pattern = std::regex(
            "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}"
            "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );
    }
    
    std::string ping(const std::string& url) {
        // Validate input
        std::string sanitized_url = url;
        // Remove whitespace
        sanitized_url.erase(
            std::remove_if(sanitized_url.begin(), sanitized_url.end(), ::isspace),
            sanitized_url.end()
        );
        
        if (!validateInput(sanitized_url)) {
            return "Invalid input: Must be a valid domain or IP address";
        }
        
        try {
            std::string command;
            #ifdef _WIN32
                command = "ping -n 4 " + sanitized_url;
            #else
                command = "ping -c 4 " + sanitized_url;
            #endif
            
            std::string output = executeCommand(command);
            return output;
            
        } catch (const std::exception& e) {
            return "Error executing ping: " + std::string(e.what());
        }
    }
};

int main() {
    Lse111 app;
    
    std::cout << "Test 1 - Valid domain:" << std::endl;
    std::cout << app.ping("google.com") << std::endl;
    
    std::cout << "\\nTest 2 - Valid IP:" << std::endl;
    std::cout << app.ping("8.8.8.8") << std::endl;
    
    std::cout << "\\nTest 3 - Invalid input (command injection attempt):" << std::endl;
    std::cout << app.ping("google.com; ls -la") << std::endl;
    
    std::cout << "\\nTest 4 - Empty input:" << std::endl;
    std::cout << app.ping("") << std::endl;
    
    std::cout << "\\nTest 5 - Invalid format:" << std::endl;
    std::cout << app.ping("not_a_valid_domain!") << std::endl;
    
    return 0;
}
