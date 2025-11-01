
#include <iostream>
#include <string>
#include <cstdlib>
#include <regex>
#include <array>
#include <memory>

class Lse113 {
private:
    static bool isValidUrl(const std::string& url) {
        if (url.empty()) {
            return false;
        }
        
        // Remove protocol prefix
        std::string cleanUrl = url;
        size_t pos = cleanUrl.find("://");
        if (pos != std::string::npos) {
            cleanUrl = cleanUrl.substr(pos + 3);
        }
        
        // Only allow alphanumeric, dots, hyphens, and colons
        std::regex pattern("^[a-zA-Z0-9.-]+(:[0-9]+)?$");
        return std::regex_match(cleanUrl, pattern);
    }
    
    static std::string extractHostname(const std::string& url) {
        std::string cleanUrl = url;
        
        // Remove protocol
        size_t pos = cleanUrl.find("://");
        if (pos != std::string::npos) {
            cleanUrl = cleanUrl.substr(pos + 3);
        }
        
        // Remove port if exists
        pos = cleanUrl.find(":");
        if (pos != std::string::npos) {
            cleanUrl = cleanUrl.substr(0, pos);
        }
        
        return cleanUrl;
    }
    
    static std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        
        if (!pipe) {
            return "Error executing command";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result;
    }

public:
    static std::string ping(const std::string& url) {
        // Validate URL to prevent command injection
        if (!isValidUrl(url)) {
            return "Invalid URL format. URL is not alive.";
        }
        
        std::string hostname = extractHostname(url);
        
        try {
            // Build ping command
            #ifdef _WIN32
                std::string command = "ping -n 1 " + hostname;
            #else
                std::string command = "ping -c 1 " + hostname;
            #endif
            
            std::string pong = exec(command.c_str());
            int exitCode = system(command.c_str());
            
            if (exitCode == 0) {
                return "URL " + url + " is alive.\\n" + pong;
            } else {
                return "URL " + url + " is not alive.";
            }
            
        } catch (const std::exception& e) {
            return "Error pinging URL " + url + ": " + e.what();
        }
    }
};

int main() {
    std::string testUrls[] = {
        "google.com",
        "127.0.0.1",
        "localhost",
        "example.com",
        "invalid-url-that-does-not-exist-12345.com"
    };
    
    std::cout << "=== Ping Test Results ===" << std::endl << std::endl;
    
    for (const auto& testUrl : testUrls) {
        std::cout << "Testing: " << testUrl << std::endl;
        std::cout << Lse113::ping(testUrl) << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
