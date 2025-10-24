
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

class Task80 {
public:
    static SSL* createSSLClientSocket(const std::string& host, int port) {
        // Initialize OpenSSL
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        
        // Create SSL context
        const SSL_METHOD* method = TLS_client_method();
        SSL_CTX* ctx = SSL_CTX_new(method);
        if (!ctx) {
            throw std::runtime_error("Unable to create SSL context");
        }
        
        // Create socket
        struct addrinfo hints, *servinfo, *p;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;  // IPv6
        hints.ai_socktype = SOCK_STREAM;  // TCP
        
        std::string portStr = std::to_string(port);
        int rv = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &servinfo);
        if (rv != 0) {
            SSL_CTX_free(ctx);
            throw std::runtime_error("getaddrinfo failed");
        }
        
        int sockfd = -1;
        for (p = servinfo; p != NULL; p = p->ai_next) {
            sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sockfd == -1) continue;
            
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) != -1) break;
            
            close(sockfd);
            sockfd = -1;
        }
        
        freeaddrinfo(servinfo);
        
        if (sockfd == -1) {
            SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to connect");
        }
        
        // Create SSL connection
        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockfd);
        
        if (SSL_connect(ssl) <= 0) {
            SSL_free(ssl);
            close(sockfd);
            SSL_CTX_free(ctx);
            throw std::runtime_error("SSL connection failed");
        }
        
        return ssl;
    }
};

int main() {
    std::string testHosts[] = {
        "www.google.com",
        "www.github.com",
        "www.amazon.com",
        "www.microsoft.com",
        "www.apple.com"
    };
    int testPorts[] = {443, 443, 443, 443, 443};
    
    for (int i = 0; i < 5; i++) {
        try {
            std::cout << "Test " << (i + 1) << ": Connecting to " 
                      << testHosts[i] << ":" << testPorts[i] << std::endl;
            SSL* ssl = Task80::createSSLClientSocket(testHosts[i], testPorts[i]);
            std::cout << "Successfully connected!" << std::endl;
            
            int sockfd = SSL_get_fd(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
            std::cout << "Connection closed.\\n" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Failed to connect: " << e.what() << "\\n" << std::endl;
        }
    }
    
    return 0;
}
