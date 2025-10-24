
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

class Task80 {
public:
    static SSL* createSSLClientSocket(const std::string& host, int port) {
        SSL_CTX* ctx = nullptr;
        SSL* ssl = nullptr;
        int sockfd = -1;
        
        try {
            // Initialize OpenSSL
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
            
            // Create SSL context
            const SSL_METHOD* method = TLS_client_method();
            ctx = SSL_CTX_new(method);
            if (!ctx) {
                throw std::runtime_error("Failed to create SSL context");
            }
            
            // Create IPv6 TCP socket
            sockfd = socket(AF_INET6, SOCK_STREAM, 0);
            if (sockfd < 0) {
                throw std::runtime_error("Failed to create socket");
            }
            
            // Get address info
            struct addrinfo hints, *result;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET6;
            hints.ai_socktype = SOCK_STREAM;
            
            std::string portStr = std::to_string(port);
            int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
            if (status != 0) {
                throw std::runtime_error("Failed to resolve hostname");
            }
            
            // Connect to server
            if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
                freeaddrinfo(result);
                throw std::runtime_error("Failed to connect");
            }
            freeaddrinfo(result);
            
            // Create SSL structure
            ssl = SSL_new(ctx);
            if (!ssl) {
                throw std::runtime_error("Failed to create SSL structure");
            }
            
            SSL_set_fd(ssl, sockfd);
            SSL_set_tlsext_host_name(ssl, host.c_str());
            
            // Perform SSL handshake
            if (SSL_connect(ssl) <= 0) {
                throw std::runtime_error("SSL handshake failed");
            }
            
            return ssl;
            
        } catch (const std::exception& e) {
            if (ssl) SSL_free(ssl);
            if (ctx) SSL_CTX_free(ctx);
            if (sockfd >= 0) close(sockfd);
            throw;
        }
    }
};

int main() {
    std::string testCases[][2] = {
        {"www.google.com", "443"},
        {"www.github.com", "443"},
        {"www.mozilla.org", "443"},
        {"www.wikipedia.org", "443"},
        {"www.cloudflare.com", "443"}
    };
    
    std::cout << "Testing SSL Client Socket with IPv6/TCP:" << std::endl;
    std::cout << "=========================================" << std::endl << std::endl;
    
    for (int i = 0; i < 5; i++) {
        std::string host = testCases[i][0];
        int port = std::stoi(testCases[i][1]);
        
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Host: " << host << ", Port: " << port << std::endl;
        
        try {
            SSL* ssl = Task80::createSSLClientSocket(host, port);
            std::cout << "Status: Connected successfully" << std::endl;
            std::cout << "Protocol: " << SSL_get_version(ssl) << std::endl;
            
            int sockfd = SSL_get_fd(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
            std::cout << "Socket closed successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Status: Failed - " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}
