
#include <iostream>
#include <string>
#include <cstdlib>
#include <array>
#include <memory>
#include <sstream>

std::string ping(const std::string& url) {
    try {
        std::string decoded_url = url;
        
        // Determine OS and set ping command accordingly
        #ifdef _WIN32
            std::string ping_command = "ping -n 1 " + decoded_url + " 2>&1";
        #else
            std::string ping_command = "ping -c 1 " + decoded_url + " 2>&1";
        #endif
        
        // Execute ping command and capture output
        std::array<char, 128> buffer;
        std::string pong;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(ping_command.c_str(), "r"), pclose);
        
        if (!pipe) {
            return "Error executing ping command";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            pong += buffer.data();
        }
        
        // Get exit status
        int exit_status = pclose(pipe.release());
        
        std::stringstream result;
        if (exit_status == 0) {
            result << "The URL " << decoded_url << " is alive\\n" << pong;
        } else {
            result << "The URL " << decoded_url << " is not alive\\n" << pong;
        }
        
        return result.str();
        
    } catch (const std::exception& e) {
        return std::string("Error pinging URL: ") + e.what();
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::cout << ping("google.com") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    std::cout << ping("localhost") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    std::cout << ping("8.8.8.8") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    std::cout << ping("invalid-url-that-does-not-exist-12345.com") << std::endl;
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    std::cout << ping("github.com") << std::endl;
    
    return 0;
}
