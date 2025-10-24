
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL* createSSLClientSocket(const char* host, int port) {
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // Create SSL context
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        return NULL;
    }
    
    // Create socket
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;  // IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP
    
    char portStr[10];
    sprintf(portStr, "%d", port);
    
    int rv = getaddrinfo(host, portStr, &hints, &servinfo);
    if (rv != 0) {
        SSL_CTX_free(ctx);
        return NULL;
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
        return NULL;
    }
    
    // Create SSL connection
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    
    if (SSL_connect(ssl) <= 0) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    return ssl;
}

int main() {
    const char* testHosts[] = {
        "www.google.com",
        "www.github.com",
        "www.amazon.com",
        "www.microsoft.com",
        "www.apple.com"
    };
    int testPorts[] = {443, 443, 443, 443, 443};
    
    for (int i = 0; i < 5; i++) {
        printf("Test %d: Connecting to %s:%d\\n", i + 1, testHosts[i], testPorts[i]);
        SSL* ssl = createSSLClientSocket(testHosts[i], testPorts[i]);
        
        if (ssl != NULL) {
            printf("Successfully connected!\\n");
            int sockfd = SSL_get_fd(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
            printf("Connection closed.\\n\\n");
        } else {
            printf("Failed to connect.\\n\\n");
        }
    }
    
    return 0;
}
