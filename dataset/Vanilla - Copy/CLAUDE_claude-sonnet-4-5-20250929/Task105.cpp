
#include <iostream>
#include <string>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

class Task105 {
public:
    static SSL* createSecureConnection(const std::string& host, int port) {
        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            throw std::runtime_error("Failed to create SSL context");
        }
        
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
        SSL_CTX_set_default_verify_paths(ctx);
        
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to create socket");
        }
        
        struct hostent* server = gethostbyname(host.c_str());
        if (!server) {
            close(sockfd);
            SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to resolve hostname");
        }
        
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sockfd);
            SSL_CTX_free(ctx);
            throw std::runtime_error("Failed to connect");
        }
        
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
    
    static std::string sendHttpsRequest(const std::string& host, int port, const std::string& request) {
        try {
            SSL* ssl = createSecureConnection(host, port);
            
            std::string fullRequest = request + "\\r\\nHost: " + host + "\\r\\nConnection: close\\r\\n\\r\\n";
            SSL_write(ssl, fullRequest.c_str(), fullRequest.length());
            
            std::string response;
            char buffer[4096];
            int bytes;
            int totalBytes = 0;
            while ((bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0 && totalBytes < 10000) {
                buffer[bytes] = '\\0';
                response += buffer;
                totalBytes += bytes;
            }
            
            int sockfd = SSL_get_fd(ssl);
            SSL_free(ssl);
            close(sockfd);
            
            return response;
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    static std::string getConnectionInfo(const std::string& host, int port) {
        try {
            SSL* ssl = createSecureConnection(host, port);
            
            std::string info;
            info += "Protocol: " + std::string(SSL_get_version(ssl)) + "\\n";
            info += "Cipher Suite: " + std::string(SSL_get_cipher(ssl)) + "\\n";
            
            int sockfd = SSL_get_fd(ssl);
            SSL_free(ssl);
            close(sockfd);
            
            return info;
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
    
    static bool verifyConnection(const std::string& host, int port) {
        try {
            SSL* ssl = createSecureConnection(host, port);
            int sockfd = SSL_get_fd(ssl);
            SSL_free(ssl);
            close(sockfd);
            return true;
        } catch (...) {
            return false;
        }
    }
};

int main() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    std::cout << "Test Case 1: Verify secure connection to google.com:443" << std::endl;
    try {
        bool result = Task105::verifyConnection("google.com", 443);
        std::cout << "Connection verified: " << (result ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 2: Get connection info for google.com:443" << std::endl;
    try {
        std::string info = Task105::getConnectionInfo("google.com", 443);
        std::cout << info << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 3: Send HTTPS request to example.com:443" << std::endl;
    try {
        std::string response = Task105::sendHttpsRequest("example.com", 443, "GET / HTTP/1.1");
        std::cout << "Response received (first 200 chars): " << response.substr(0, 200) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 4: Verify connection to httpbin.org:443" << std::endl;
    try {
        bool result = Task105::verifyConnection("httpbin.org", 443);
        std::cout << "Connection verified: " << (result ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 5: Test invalid connection" << std::endl;
    try {
        bool result = Task105::verifyConnection("invalid-host-12345.com", 443);
        std::cout << "Connection verified: " << (result ? "true" : "false") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Expected failure: " << e.what() << std::endl;
    }
    
    EVP_cleanup();
    ERR_free_strings();
    
    return 0;
}
