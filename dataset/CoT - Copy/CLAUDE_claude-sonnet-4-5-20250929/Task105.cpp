
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

class Task105 {
public:
    /**
     * Establishes a secure SSL/TLS connection with a remote server
     */
    static std::string establishSecureConnection(const std::string& hostname, int port, const std::string& message = "") {
        if (hostname.empty()) {
            return "Error: Invalid hostname";
        }
        
        if (port < 1 || port > 65535) {
            return "Error: Invalid port number";
        }
        
        SSL_CTX* ctx = nullptr;
        SSL* ssl = nullptr;
        int sock = -1;
        
        try {
            // Initialize OpenSSL
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
            
            // Create SSL context with secure protocol
            const SSL_METHOD* method = TLS_client_method();
            ctx = SSL_CTX_new(method);
            
            if (!ctx) {
                return "Error: Failed to create SSL context";
            }
            
            // Set minimum TLS version to 1.2
            SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
            
            // Load default CA certificates
            if (!SSL_CTX_set_default_verify_paths(ctx)) {
                SSL_CTX_free(ctx);
                return "Error: Failed to load CA certificates";
            }
            
            // Enable certificate verification
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
            
            // Set secure cipher list
            SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4");
            
            // Create socket
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                SSL_CTX_free(ctx);
                return "Error: Failed to create socket";
            }
            
            // Set socket timeout
            struct timeval timeout;
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
            
            // Resolve hostname
            struct hostent* host = gethostbyname(hostname.c_str());
            if (!host) {
                close(sock);
                SSL_CTX_free(ctx);
                return "Error: Failed to resolve hostname";
            }
            
            // Connect to server
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);
            
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                close(sock);
                SSL_CTX_free(ctx);
                return "Error: Failed to connect to server";
            }
            
            // Create SSL structure
            ssl = SSL_new(ctx);
            if (!ssl) {
                close(sock);
                SSL_CTX_free(ctx);
                return "Error: Failed to create SSL structure";
            }
            
            // Set hostname for SNI
            SSL_set_tlsext_host_name(ssl, hostname.c_str());
            
            // Attach socket to SSL
            SSL_set_fd(ssl, sock);
            
            // Perform SSL handshake
            if (SSL_connect(ssl) <= 0) {
                SSL_free(ssl);
                close(sock);
                SSL_CTX_free(ctx);
                return "Error: SSL handshake failed";
            }
            
            // Verify certificate
            if (SSL_get_verify_result(ssl) != X509_V_OK) {
                SSL_free(ssl);
                close(sock);
                SSL_CTX_free(ctx);
                return "Error: Certificate verification failed";
            }
            
            std::string result = "Connection successful. Protocol: ";
            result += SSL_get_version(ssl);
            result += ", Cipher: ";
            result += SSL_get_cipher(ssl);
            result += "\\n";
            
            // Send message if provided
            if (!message.empty()) {
                int written = SSL_write(ssl, message.c_str(), message.length());
                if (written <= 0) {
                    result += "Warning: Failed to send message\\n";
                } else {
                    // Read response
                    char buffer[4096];
                    int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
                    if (bytes > 0) {
                        buffer[bytes] = '\\0';
                        result += "Response: ";
                        result += buffer;
                    }
                }
            }
            
            // Cleanup
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sock);
            SSL_CTX_free(ctx);
            
            return result;
            
        } catch (...) {
            if (ssl) SSL_free(ssl);
            if (sock >= 0) close(sock);
            if (ctx) SSL_CTX_free(ctx);
            return "Error: Exception occurred";
        }
    }
};

int main() {
    std::cout << "=== SSL/TLS Secure Connection Test Cases ===\\n\\n";
    
    // Test Case 1: Valid connection to Google (HTTPS port)
    std::cout << "Test Case 1: Valid HTTPS connection\\n";
    std::string result1 = Task105::establishSecureConnection("www.google.com", 443, "GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\n\\r\\n");
    std::cout << result1.substr(0, std::min(size_t(200), result1.length())) << "...\\n\\n";
    
    // Test Case 2: Empty hostname
    std::cout << "Test Case 2: Empty hostname\\n";
    std::string result2 = Task105::establishSecureConnection("", 443);
    std::cout << result2 << "\\n\\n";
    
    // Test Case 3: Invalid port
    std::cout << "Test Case 3: Invalid port\\n";
    std::string result3 = Task105::establishSecureConnection("www.google.com", 99999);
    std::cout << result3 << "\\n\\n";
    
    // Test Case 4: Low port number
    std::cout << "Test Case 4: Invalid low port\\n";
    std::string result4 = Task105::establishSecureConnection("www.google.com", 0);
    std::cout << result4 << "\\n\\n";
    
    // Test Case 5: Invalid hostname
    std::cout << "Test Case 5: Invalid hostname\\n";
    std::string result5 = Task105::establishSecureConnection("invalid.hostname.xyz", 443);
    std::cout << result5 << "\\n\\n";
    
    return 0;
}
