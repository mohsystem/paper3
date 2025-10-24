
#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// RAII wrapper for SSL context
class SSLContextWrapper {
private:
    SSL_CTX* ctx;
public:
    SSLContextWrapper() : ctx(nullptr) {
        // Initialize OpenSSL library
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        
        // Use TLS 1.2 or higher (Rules#6: enforce TLS 1.2+)
        const SSL_METHOD* method = TLS_client_method();
        if (!method) {
            throw std::runtime_error("Failed to get TLS method");
        }
        
        ctx = SSL_CTX_new(method);
        if (!ctx) {
            throw std::runtime_error("Failed to create SSL context");
        }
        
        // Set minimum TLS version to 1.2 (Rules#6)
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        
        // Enable certificate verification (Rules#3)
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        
        // Load default trusted CA certificates (Rules#3)
        if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
            SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to load CA certificates");
        }
    }
    
    ~SSLContextWrapper() {
        if (ctx) {
            SSL_CTX_free(ctx);
        }
    }
    
    SSL_CTX* get() { return ctx; }
};

// RAII wrapper for SSL connection
class SSLWrapper {
private:
    SSL* ssl;
public:
    explicit SSLWrapper(SSL_CTX* ctx) : ssl(nullptr) {
        if (!ctx) {
            throw std::runtime_error("Invalid SSL context");
        }
        ssl = SSL_new(ctx);
        if (!ssl) {
            throw std::runtime_error("Failed to create SSL object");
        }
    }
    
    ~SSLWrapper() {
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
    }
    
    SSL* get() { return ssl; }
};

// Function to validate input parameters (Rules#1, Rules#10)
void validateInputs(const std::string& host, int port) {
    // Validate host is not empty (Rules#1: validate all inputs)
    if (host.empty() || host.length() > 255) {
        throw std::invalid_argument("Invalid host: must be 1-255 characters");
    }
    
    // Validate port range (Rules#1: validate ranges)
    if (port < 1 || port > 65535) {
        throw std::invalid_argument("Invalid port: must be between 1 and 65535");
    }
    
    // Check for null bytes in host (Rules#10: input validation)
    if (host.find('\\0') != std::string::npos) {
        throw std::invalid_argument("Invalid host: contains null bytes");
    }
}

// Function to create and connect IPv6 SSL socket
SSL* createSSLClientSocket(const std::string& host, int port) {
    // Validate inputs before processing (Rules#1, Rules#10)
    validateInputs(host, port);
    
    // Create SSL context with RAII (Rules#1: safe resource management)
    static SSLContextWrapper ctxWrapper;
    SSL_CTX* ctx = ctxWrapper.get();
    
    // Create socket for IPv6 TCP (Rules#1: specified protocol requirements)
    int sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create IPv6 socket");
    }
    
    // Use getaddrinfo for safe hostname resolution (Rules#1, Rules#10)
    struct addrinfo hints, *result = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;      // IPv6 only
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP;
    
    std::string portStr = std::to_string(port);
    int ret = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (ret != 0) {
        close(sockfd);
        throw std::runtime_error("Failed to resolve hostname: " + std::string(gai_strerror(ret)));
    }
    
    // Ensure result is freed even on exception (Rules#1: RAII pattern)
    std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> resultPtr(result, freeaddrinfo);
    
    // Connect to the server (Rules#1: check return values)
    if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to connect to server");
    }
    
    // Create SSL object with RAII wrapper (Rules#1: safe resource management)
    SSLWrapper sslWrapper(ctx);
    SSL* ssl = sslWrapper.get();
    
    // Set the socket file descriptor
    if (SSL_set_fd(ssl, sockfd) != 1) {
        close(sockfd);
        throw std::runtime_error("Failed to set socket to SSL");
    }
    
    // Enable hostname verification (Rules#4: hostname verification)
    SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (SSL_set1_host(ssl, host.c_str()) != 1) {
        close(sockfd);
        throw std::runtime_error("Failed to set hostname for verification");
    }
    
    // Perform SSL handshake (Rules#3: establish SSL connection)
    if (SSL_connect(ssl) != 1) {
        unsigned long err = ERR_get_error();
        char errBuf[256];
        ERR_error_string_n(err, errBuf, sizeof(errBuf));
        close(sockfd);
        throw std::runtime_error("SSL handshake failed: " + std::string(errBuf));
    }
    
    // Verify the server certificate (Rules#3: certificate validation)
    long verifyResult = SSL_get_verify_result(ssl);
    if (verifyResult != X509_V_OK) {
        close(sockfd);
        throw std::runtime_error("Certificate verification failed: " + 
                                std::string(X509_verify_cert_error_string(verifyResult)));
    }
    
    // Return the established SSL connection
    // Note: Caller is responsible for SSL_shutdown, SSL_free, and close(sockfd)
    return ssl;
}

int main() {
    try {
        // Test case 1: Connect to a public IPv6 HTTPS server
        std::cout << "Test 1: Attempting connection to ipv6.google.com:443" << std::endl;
        try {
            SSL* ssl1 = createSSLClientSocket("ipv6.google.com", 443);
            std::cout << "Test 1: SUCCESS - Connected" << std::endl;
            SSL_shutdown(ssl1);
            int fd1 = SSL_get_fd(ssl1);
            SSL_free(ssl1);
            close(fd1);
        } catch (const std::exception& e) {
            std::cout << "Test 1: EXPECTED FAILURE (no IPv6 or network) - " << e.what() << std::endl;
        }
        
        // Test case 2: Invalid port (too low)
        std::cout << "\\nTest 2: Invalid port 0" << std::endl;
        try {
            SSL* ssl2 = createSSLClientSocket("example.com", 0);
            std::cout << "Test 2: UNEXPECTED SUCCESS" << std::endl;
            SSL_shutdown(ssl2);
            int fd2 = SSL_get_fd(ssl2);
            SSL_free(ssl2);
            close(fd2);
        } catch (const std::invalid_argument& e) {
            std::cout << "Test 2: SUCCESS - Caught invalid port: " << e.what() << std::endl;
        }
        
        // Test case 3: Invalid port (too high)
        std::cout << "\\nTest 3: Invalid port 70000" << std::endl;
        try {
            SSL* ssl3 = createSSLClientSocket("example.com", 70000);
            std::cout << "Test 3: UNEXPECTED SUCCESS" << std::endl;
            SSL_shutdown(ssl3);
            int fd3 = SSL_get_fd(ssl3);
            SSL_free(ssl3);
            close(fd3);
        } catch (const std::invalid_argument& e) {
            std::cout << "Test 3: SUCCESS - Caught invalid port: " << e.what() << std::endl;
        }
        
        // Test case 4: Empty hostname
        std::cout << "\\nTest 4: Empty hostname" << std::endl;
        try {
            SSL* ssl4 = createSSLClientSocket("", 443);
            std::cout << "Test 4: UNEXPECTED SUCCESS" << std::endl;
            SSL_shutdown(ssl4);
            int fd4 = SSL_get_fd(ssl4);
            SSL_free(ssl4);
            close(fd4);
        } catch (const std::invalid_argument& e) {
            std::cout << "Test 4: SUCCESS - Caught empty hostname: " << e.what() << std::endl;
        }
        
        // Test case 5: Very long hostname
        std::cout << "\\nTest 5: Hostname too long (>255 chars)" << std::endl;
        try {
            std::string longHost(300, 'a');
            SSL* ssl5 = createSSLClientSocket(longHost, 443);
            std::cout << "Test 5: UNEXPECTED SUCCESS" << std::endl;
            SSL_shutdown(ssl5);
            int fd5 = SSL_get_fd(ssl5);
            SSL_free(ssl5);
            close(fd5);
        } catch (const std::invalid_argument& e) {
            std::cout << "Test 5: SUCCESS - Caught invalid hostname: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
