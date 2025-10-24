
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

class Task105 {
public:
    static SSL* establishSecureConnection(const std::string& host, int port) {
        // Initialize OpenSSL
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        
        // Create SSL context with TLSv1.2 or higher
        const SSL_METHOD* method = TLS_client_method();
        SSL_CTX* ctx = SSL_CTX_new(method);
        
        if (!ctx) {
            throw std::runtime_error("Unable to create SSL context");
        }
        
        // Set minimum protocol version to TLSv1.2
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        
        // Set secure cipher list
        SSL_CTX_set_cipher_list(ctx, "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:!aNULL:!MD5:!DSS");
        
        // Enable certificate verification
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        SSL_CTX_set_default_verify_paths(ctx);
        
        // Create socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            SSL_CTX_free(ctx);
            throw std::runtime_error("Unable to create socket");
        }
        
        // Resolve hostname
        struct hostent* server = gethostbyname(host.c_str());
        if (!server) {
            close(sockfd);
            SSL_CTX_free(ctx);
            throw std::runtime_error("Host not found");
        }
        
        // Connect to server
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sockfd);
            SSL_CTX_free(ctx);
            throw std::runtime_error("Connection failed");
        }
        
        // Create SSL connection
        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sockfd);
        SSL_set_tlsext_host_name(ssl, host.c_str());
        
        if (SSL_connect(ssl) <= 0) {
            SSL_free(ssl);
            close(sockfd);
            SSL_CTX_free(ctx);
            throw std::runtime_error("SSL handshake failed");
        }
        
        return ssl;
    }
    
    static std::string sendRequest(const std::string& host, int port, const std::string& request) {
        SSL* ssl = establishSecureConnection(host, port);
        
        // Send request
        SSL_write(ssl, request.c_str(), request.length());
        
        // Receive response
        std::string response;
        char buffer[4096];
        int bytes;
        
        while ((bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes] = '\\0';
            response += buffer;
            if (response.find("\\r\\n\\r\\n") != std::string::npos) {
                break;
            }
            if (response.length() > 10000) {
                break;
            }
        }
        
        // Cleanup
        int sockfd = SSL_get_fd(ssl);
        SSL_free(ssl);
        close(sockfd);
        
        return response;
    }
};

int main() {
    std::cout << "SSL/TLS Secure Connection Test Cases:\\n\\n";
    
    // Test case 1
    try {
        std::cout << "Test 1: Connecting to www.google.com:443\\n";
        std::string response = Task105::sendRequest("www.google.com", 443, 
            "GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\n\\r\\n");
        std::cout << "Connection successful! Response length: " << response.length() << "\\n";
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << "\\n";
    }
    
    // Test case 2
    try {
        std::cout << "\\nTest 2: Connecting to github.com:443\\n";
        std::string response = Task105::sendRequest("github.com", 443, 
            "GET / HTTP/1.1\\r\\nHost: github.com\\r\\n\\r\\n");
        std::cout << "Connection successful! Response length: " << response.length() << "\\n";
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << "\\n";
    }
    
    // Test case 3
    try {
        std::cout << "\\nTest 3: Connecting to www.amazon.com:443\\n";
        std::string response = Task105::sendRequest("www.amazon.com", 443, 
            "GET / HTTP/1.1\\r\\nHost: www.amazon.com\\r\\n\\r\\n");
        std::cout << "Connection successful! Response length: " << response.length() << "\\n";
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << "\\n";
    }
    
    // Test case 4
    try {
        std::cout << "\\nTest 4: Connecting to www.microsoft.com:443\\n";
        std::string response = Task105::sendRequest("www.microsoft.com", 443, 
            "GET / HTTP/1.1\\r\\nHost: www.microsoft.com\\r\\n\\r\\n");
        std::cout << "Connection successful! Response length: " << response.length() << "\\n";
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << "\\n";
    }
    
    // Test case 5
    try {
        std::cout << "\\nTest 5: Connecting to invalid.host.example:443\\n";
        std::string response = Task105::sendRequest("invalid.host.example", 443, 
            "GET / HTTP/1.1\\r\\nHost: invalid.host.example\\r\\n\\r\\n");
        std::cout << "Connection successful! Response length: " << response.length() << "\\n";
    } catch (const std::exception& e) {
        std::cout << "Test 5 failed (expected): " << e.what() << "\\n";
    }
    
    return 0;
}
