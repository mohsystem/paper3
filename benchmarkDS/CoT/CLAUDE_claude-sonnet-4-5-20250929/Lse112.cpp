
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

class Lse112 {
private:
    static bool isValidHostname(const std::string& hostname) {
        std::regex pattern("^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)*[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$");
        return std::regex_match(hostname, pattern);
    }

public:
    static std::string securePing(const std::string& url) {
        if (url.empty()) {
            return "Error: URL parameter is required";
        }
        
        if (!isValidHostname(url)) {
            return "Error: Invalid hostname format";
        }
        
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        int status = getaddrinfo(url.c_str(), "80", &hints, &res);
        if (status != 0) {
            return "Error: Unknown host - " + url;
        }
        
        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) {
            freeaddrinfo(res);
            return "Error: Unable to create socket";
        }
        
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        
        bool reachable = (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0);
        
        close(sockfd);
        freeaddrinfo(res);
        
        return "Ping result for " + url + ": " + (reachable ? "Host is reachable" : "Host found but not reachable on port 80");
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse112::securePing("google.com") << std::endl;
    std::cout << "Test 2: " << Lse112::securePing("localhost") << std::endl;
    std::cout << "Test 3: " << Lse112::securePing("127.0.0.1") << std::endl;
    std::cout << "Test 4: " << Lse112::securePing("invalid..host") << std::endl;
    std::cout << "Test 5: " << Lse112::securePing("; rm -rf /") << std::endl;
    
    std::cout << "\\nNote: Full web server implementation requires additional libraries like libmicrohttpd or Boost.Beast" << std::endl;
    
    return 0;
}
