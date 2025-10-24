
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define TIMEOUT 5
#define MAX_RESPONSE_SIZE 1048576

class Task160 {
public:
    static std::string makeHttpRequest(const std::string& urlString) {
        if (urlString.empty()) {
            return "Error: URL cannot be empty";
        }
        
        // Parse URL
        std::string protocol, host, path;
        int port;
        if (!parseURL(urlString, protocol, host, port, path)) {
            return "Error: Invalid URL format";
        }
        
        // Security: Only allow HTTP and HTTPS
        if (protocol != "http" && protocol != "https") {
            return "Error: Only HTTP and HTTPS protocols are allowed";
        }
        
        // Security: Block private/local addresses
        if (isPrivateOrLocalAddress(host)) {
            return "Error: Requests to private/internal addresses are not allowed";
        }
        
        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return "Error: Failed to create socket";
        }
        
        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        // Resolve hostname
        struct hostent* server = gethostbyname(host.c_str());
        if (server == NULL) {
            close(sock);
            return "Error: Failed to resolve hostname";
        }
        
        // Connect to server
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        
        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock);
            return "Error: Failed to connect";
        }
        
        // Send HTTP request
        std::string request = "GET " + path + " HTTP/1.1\\r\\n";
        request += "Host: " + host + "\\r\\n";
        request += "User-Agent: SecureHttpClient/1.0\\r\\n";
        request += "Connection: close\\r\\n\\r\\n";
        
        if (send(sock, request.c_str(), request.length(), 0) < 0) {
            close(sock);
            return "Error: Failed to send request";
        }
        
        // Read response
        std::string response;
        char buffer[4096];
        int bytesRead;
        int totalBytes = 0;
        
        while ((bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            totalBytes += bytesRead;
            if (totalBytes > MAX_RESPONSE_SIZE) {
                close(sock);
                return "Error: Response size exceeds limit";
            }
            buffer[bytesRead] = '\\0';
            response += buffer;
        }
        
        close(sock);
        return response;
    }
    
private:
    static bool parseURL(const std::string& url, std::string& protocol, 
                        std::string& host, int& port, std::string& path) {
        std::regex urlPattern("^(https?)://([^:/]+)(?::([0-9]+))?(/.*)?$");
        std::smatch matches;
        
        if (!std::regex_match(url, matches, urlPattern)) {
            return false;
        }
        
        protocol = matches[1].str();
        host = matches[2].str();
        port = matches[3].str().empty() ? 
               (protocol == "https" ? 443 : 80) : std::stoi(matches[3].str());
        path = matches[4].str().empty() ? "/" : matches[4].str();
        
        return true;
    }
    
    static bool isPrivateOrLocalAddress(const std::string& host) {
        // Block localhost
        if (host == "localhost" || host == "127.0.0.1" || host == "0.0.0.0") {
            return true;
        }
        
        // Check if it's a private IP
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        
        if (getaddrinfo(host.c_str(), NULL, &hints, &res) == 0) {
            struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
            unsigned char* ip = (unsigned char*)&addr->sin_addr.s_addr;
            
            bool isPrivate = (ip[0] == 10) ||
                           (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) ||
                           (ip[0] == 192 && ip[1] == 168) ||
                           (ip[0] == 127) ||
                           (ip[0] == 169 && ip[1] == 254);
            
            freeaddrinfo(res);
            return isPrivate;
        }
        
        return false;
    }
};

int main() {
    std::cout << "=== Test Case 1: Valid HTTP URL ===" << std::endl;
    std::cout << Task160::makeHttpRequest("http://www.example.com") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Invalid Protocol ===" << std::endl;
    std::cout << Task160::makeHttpRequest("ftp://example.com") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Localhost (blocked) ===" << std::endl;
    std::cout << Task160::makeHttpRequest("http://localhost:8080") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Private IP (blocked) ===" << std::endl;
    std::cout << Task160::makeHttpRequest("http://192.168.1.1") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Empty URL ===" << std::endl;
    std::cout << Task160::makeHttpRequest("") << std::endl;
    
    return 0;
}
