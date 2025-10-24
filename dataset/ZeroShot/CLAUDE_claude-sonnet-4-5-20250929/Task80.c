
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL* createSSLClientSocket(const char* host, int port) {
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    int sockfd = -1;
    
    /* Initialize OpenSSL */
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    /* Create SSL context */
    const SSL_METHOD* method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        fprintf(stderr, "Failed to create SSL context\\n");
        return NULL;
    }
    
    /* Create IPv6 TCP socket */
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Failed to create socket\\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    /* Get address info */
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    
    char portStr[10];
    snprintf(portStr, sizeof(portStr), "%d", port);
    
    int status = getaddrinfo(host, portStr, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "Failed to resolve hostname\\n");
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    /* Connect to server */
    if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        fprintf(stderr, "Failed to connect\\n");
        freeaddrinfo(result);
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    freeaddrinfo(result);
    
    /* Create SSL structure */
    ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Failed to create SSL structure\\n");
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);
    
    /* Perform SSL handshake */
    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "SSL handshake failed\\n");
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
        "www.mozilla.org",
        "www.wikipedia.org",
        "www.cloudflare.com"
    };
    int testPorts[] = {443, 443, 443, 443, 443};
    
    printf("Testing SSL Client Socket with IPv6/TCP:\\n");
    printf("=========================================\\n\\n");
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        printf("Host: %s, Port: %d\\n", testHosts[i], testPorts[i]);
        
        SSL* ssl = createSSLClientSocket(testHosts[i], testPorts[i]);
        if (ssl) {
            printf("Status: Connected successfully\\n");
            printf("Protocol: %s\\n", SSL_get_version(ssl));
            
            int sockfd = SSL_get_fd(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
            printf("Socket closed successfully\\n");
        } else {
            printf("Status: Failed\\n");
        }
        printf("\\n");
    }
    
    return 0;
}
