
// C++ - SECURE VERSION with input validation
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

class Lse111 {
private:
    static bool isValidHost(const std::string& url) {
        // Whitelist pattern for valid hostnames/IPs
        std::regex validPattern(
            "^([a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?\\\\.)*"
            "[a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?$|"
            "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}"
            "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
        );
        return std::regex_match(url, validPattern);
    }
    
    static bool checkConnection(const std::string& host, int port) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;
        
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        struct hostent* server = gethostbyname(host.c_str());
        if (server == nullptr) {
            close(sock);
            return false;
        }
        
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        
        bool result = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0;
        close(sock);
        return result;
    }

public:
    static std::string ping(const std::string& url) {
        if (url.empty()) {
            return "Error: URL cannot be empty";
        }
        
        if (!isValidHost(url)) {
            return "Error: Invalid hostname or IP address format";
        }
        
        std::string output = "Ping test for: " + url + "\\n";
        
        struct hostent* host = gethostbyname(url.c_str());
        if (host == nullptr) {
            return output + "Error: Unknown host - " + url;
        }
        
        struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
        if (addr_list[0] != nullptr) {
            std::string ip = inet_ntoa(*addr_list[0]);
            output += "Host: " + ip + "\\n";
            
            // Try common ports
            int ports[] = {80, 443, 22};
            bool reachable = false;
            
            for (int port : ports) {
                if (checkConnection(url, port)) {
                    output += "Port " + std::to_string(port) + ": Open\\n";
                    reachable = true;
                    break;
                }
            }
            
            output += "Reachable: " + std::string(reachable ? "Yes" : "No") + "\\n";
        }
        
        return output;
    }
};

int main() {
    // Test cases
    std::cout << "=== Test Case 1: Valid domain ===" << std::endl;
    std::cout << Lse111::ping("google.com") << std::endl;
    
    std::cout << "=== Test Case 2: Valid IP ===" << std::endl;
    std::cout << Lse111::ping("8.8.8.8") << std::endl;
    
    std::cout << "=== Test Case 3: Localhost ===" << std::endl;
    std::cout << Lse111::ping("127.0.0.1") << std::endl;
    
    std::cout << "=== Test Case 4: Invalid input (command injection attempt) ===" << std::endl;
    std::cout << Lse111::ping("google.com; rm -rf /") << std::endl;
    
    std::cout << "=== Test Case 5: Empty input ===" << std::endl;
    std::cout << Lse111::ping("") << std::endl;
    
    return 0;
}
