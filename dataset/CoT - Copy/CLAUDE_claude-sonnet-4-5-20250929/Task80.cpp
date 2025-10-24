
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

class SSLClientSocket {
private:
    SSL_CTX* ctx;
    SSL* ssl;
    int sockfd;
    
public:
    SSLClientSocket() : ctx(nullptr), ssl(nullptr), sockfd(-1) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
    }
    
    ~SSLClientSocket() {
        cleanup();
    }
    
    SSL* createSSLClientSocket(const std::string& host, int port) {
        if (host.empty()) {
            throw std::invalid_argument("Host cannot be empty");
        }
        if (port < 1 || port > 65535) {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }
        
        // Create SSL context with TLS 1.2 or higher
        const SSL_METHOD* method = TLS_client_method();
        ctx = SSL_CTX_new(method);
        if (!ctx) {
            throw std::runtime_error("Failed to create SSL context");
        }
        
        // Set minimum TLS version
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        
        // Load default CA certificates
        SSL_CTX_set_default_verify_paths(ctx);
        
        // Enable certificate verification
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        
        // Create socket with IPv6 support
        sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0) {
            // Fallback to IPv4
            sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sockfd < 0) {
                throw std::runtime_error("Failed to create socket");
            }
        }
        
        // Set socket timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        // Resolve hostname
        struct addrinfo hints, *result;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        std::string portStr = std::to_string(port);
        int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
        if (status != 0) {
            throw std::runtime_error("Failed to resolve hostname");
        }
        
        // Connect to host
        if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
            freeaddrinfo(result);
            throw std::runtime_error("Failed to connect to host");
        }
        freeaddrinfo(result);
        
        // Create SSL structure
        ssl = SSL_new(ctx);
        if (!ssl) {
            throw std::runtime_error("Failed to create SSL structure");
        }
        
        // Enable hostname verification
        SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
        if (!SSL_set1_host(ssl, host.c_str())) {
            throw std::runtime_error("Failed to set hostname");
        }
        
        // Attach socket to SSL
        SSL_set_fd(ssl, sockfd);
        
        // Perform SSL handshake
        if (SSL_connect(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            throw std::runtime_error("SSL handshake failed");
        }
        
        // Verify certificate
        if (SSL_get_verify_result(ssl) != X509_V_OK) {
            throw std::runtime_error("Certificate verification failed");
        }
        
        return ssl;
    }
    
    void cleanup() {
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            ssl = nullptr;
        }
        if (sockfd >= 0) {
            close(sockfd);
            sockfd = -1;
        }
        if (ctx) {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
    }
};

int main() {
    std::string testCases[][2] = {
        {"www.google.com", "443"},
        {"www.github.com", "443"},
        {"www.amazon.com", "443"},
        {"www.microsoft.com", "443"},
        {"www.cloudflare.com", "443"}
    };
    
    for (const auto& testCase : testCases) {
        try {
            std::string host = testCase[0];
            int port = std::stoi(testCase[1]);
            
            std::cout << "Connecting to " << host << ":" << port << std::endl;
            
            SSLClientSocket client;
            SSL* ssl = client.createSSLClientSocket(host, port);
            
            if (ssl) {
                std::cout << "Successfully connected to " << host << ":" << port << std::endl;
                std::cout << "Protocol: " << SSL_get_version(ssl) << std::endl;
                std::cout << "Cipher: " << SSL_get_cipher(ssl) << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "Failed to connect: " << e.what() << std::endl;
        }
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
