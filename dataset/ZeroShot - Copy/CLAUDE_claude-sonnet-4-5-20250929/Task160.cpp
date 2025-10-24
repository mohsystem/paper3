
#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

class Task160 {
private:
    static const int TIMEOUT = 5;
    static const int MAX_RESPONSE_SIZE = 1048576;
    
    static std::set<std::string> getBlockedHosts() {
        return {"localhost", "127.0.0.1", "0.0.0.0", "::1", "169.254.169.254"};
    }
    
    static std::string toLowerCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    static bool startsWith(const std::string& str, const std::string& prefix) {
        return str.substr(0, prefix.length()) == prefix;
    }
    
    static bool isBlockedHost(const std::string& host) {
        std::set<std::string> blocked = getBlockedHosts();
        std::string lowerHost = toLowerCase(host);
        
        if (blocked.find(lowerHost) != blocked.end()) return true;
        if (startsWith(lowerHost, "192.168.")) return true;
        if (startsWith(lowerHost, "10.")) return true;
        if (startsWith(lowerHost, "172.")) return true;
        
        return false;
    }
    
    static std::string extractHost(const std::string& url) {
        size_t start = url.find("://");
        if (start == std::string::npos) return "";
        start += 3;
        
        size_t end = url.find("/", start);
        if (end == std::string::npos) end = url.length();
        
        size_t portPos = url.find(":", start);
        if (portPos != std::string::npos && portPos < end) {
            end = portPos;
        }
        
        return url.substr(start, end - start);
    }

public:
    static std::string makeHttpRequest(const std::string& urlString) {
        // Basic URL validation
        if (urlString.empty()) {
            return "Error: Empty URL";
        }
        
        // Check protocol
        std::string lowerUrl = toLowerCase(urlString);
        if (!startsWith(lowerUrl, "http://") && !startsWith(lowerUrl, "https://")) {
            return "Error: Only HTTP and HTTPS protocols are allowed";
        }
        
        // Extract and validate host
        std::string host = extractHost(urlString);
        if (host.empty()) {
            return "Error: Invalid URL - no hostname";
        }
        
        if (isBlockedHost(host)) {
            return "Error: Access to internal/private network addresses is blocked";
        }
        
        return "Note: Basic validation passed. Full HTTP implementation requires external libraries like libcurl.\\n"
               "URL: " + urlString + "\\nHost: " + host + "\\n"
               "In production, use libcurl or similar library for actual HTTP requests.";
    }
};

int main() {
    std::cout << "HTTP Request Maker - Test Cases\\n\\n";
    
    // Test case 1: Valid HTTPS URL
    std::cout << "Test 1: Valid HTTPS URL\\n";
    std::string result1 = Task160::makeHttpRequest("https://www.example.com");
    std::cout << result1 << "\\n\\n";
    
    // Test case 2: Invalid protocol
    std::cout << "Test 2: Invalid protocol (file://)\\n";
    std::string result2 = Task160::makeHttpRequest("file:///etc/passwd");
    std::cout << result2 << "\\n\\n";
    
    // Test case 3: Blocked host (localhost)
    std::cout << "Test 3: Blocked host (localhost)\\n";
    std::string result3 = Task160::makeHttpRequest("http://localhost:8080");
    std::cout << result3 << "\\n\\n";
    
    // Test case 4: Blocked private IP
    std::cout << "Test 4: Blocked private IP\\n";
    std::string result4 = Task160::makeHttpRequest("http://192.168.1.1");
    std::cout << result4 << "\\n\\n";
    
    // Test case 5: Invalid URL
    std::cout << "Test 5: Invalid URL\\n";
    std::string result5 = Task160::makeHttpRequest("not-a-valid-url");
    std::cout << result5 << "\\n\\n";
    
    return 0;
}
