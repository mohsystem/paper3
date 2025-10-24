
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

typedef struct {
    SSL_CTX* ctx;
    SSL* ssl;
    int sockfd;
} SSLClientContext;

SSL* create_ssl_client_socket(const char* host, int port, SSLClientContext* context) {
    if (!host || strlen(host) == 0) {
        fprintf(stderr, "Error: Host cannot be empty\\n");
        return NULL;
    }
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Error: Port must be between 1 and 65535\\n");
        return NULL;
    }
    
    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    // Create SSL context with TLS 1.2 or higher
    const SSL_METHOD* method = TLS_client_method();
    context->ctx = SSL_CTX_new(method);
    if (!context->ctx) {
        fprintf(stderr, "Error: Failed to create SSL context\\n");
        return NULL;
    }
    
    // Set minimum TLS version
    SSL_CTX_set_min_proto_version(context->ctx, TLS1_2_VERSION);
    
    // Load default CA certificates
    SSL_CTX_set_default_verify_paths(context->ctx);
    
    // Enable certificate verification
    SSL_CTX_set_verify(context->ctx, SSL_VERIFY_PEER, NULL);
    
    // Create socket with IPv6 support
    context->sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (context->sockfd < 0) {
        // Fallback to IPv4
        context->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (context->sockfd < 0) {
            fprintf(stderr, "Error: Failed to create socket\\n");
            SSL_CTX_free(context->ctx);
            return NULL;
        }
    }
    
    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(context->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(context->sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    // Resolve hostname
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    int status = getaddrinfo(host, port_str, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "Error: Failed to resolve hostname\\n");
        close(context->sockfd);
        SSL_CTX_free(context->ctx);
        return NULL;
    }
    
    // Connect to host
    if (connect(context->sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        fprintf(stderr, "Error: Failed to connect to host\\n");
        freeaddrinfo(result);
        close(context->sockfd);
        SSL_CTX_free(context->ctx);
        return NULL;
    }
    freeaddrinfo(result);
    
    // Create SSL structure
    context->ssl = SSL_new(context->ctx);
    if (!context->ssl) {
        fprintf(stderr, "Error: Failed to create SSL structure\\n");
        close(context->sockfd);
        SSL_CTX_free(context->ctx);
        return NULL;
    }
    
    // Enable hostname verification
    SSL_set_hostflags(context->ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!SSL_set1_host(context->ssl, host)) {
        fprintf(stderr, "Error: Failed to set hostname\\n");
        SSL_free(context->ssl);
        close(context->sockfd);
        SSL_CTX_free(context->ctx);
        return NULL;
    }
    
    // Attach socket to SSL
    SSL_set_fd(context->ssl, context->sockfd);
    
    // Perform SSL handshake
    if (SSL_connect(context->ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_free(context->ssl);
        close(context->sockfd);
        SSL_CTX_free(context->ctx);
        return NULL;
    }
    
    // Verify certificate
    if (SSL_get_verify_result(context->ssl) != X509_V_OK) {
        fprintf(stderr, "Error: Certificate verification failed\\n");
        SSL_shutdown(context->ssl);
        SSL_free(context->ssl);
        close(context->sockfd);
        SSL_CTX_free(context->ctx);
        return NULL;
    }
    
    return context->ssl;
}

void cleanup_ssl_context(SSLClientContext* context) {
    if (context->ssl) {
        SSL_shutdown(context->ssl);
        SSL_free(context->ssl);
    }
    if (context->sockfd >= 0) {
        close(context->sockfd);
    }
    if (context->ctx) {
        SSL_CTX_free(context->ctx);
    }
}

int main() {
    const char* test_hosts[] = {
        "www.google.com",
        "www.github.com",
        "www.amazon.com",
        "www.microsoft.com",
        "www.cloudflare.com"
    };
    int test_port = 443;
    
    for (int i = 0; i < 5; i++) {
        printf("Connecting to %s:%d\\n", test_hosts[i], test_port);
        
        SSLClientContext context = {NULL, NULL, -1};
        SSL* ssl = create_ssl_client_socket(test_hosts[i], test_port, &context);
        
        if (ssl) {
            printf("Successfully connected to %s:%d\\n", test_hosts[i], test_port);
            printf("Protocol: %s\\n", SSL_get_version(ssl));
            printf("Cipher: %s\\n", SSL_get_cipher(ssl));
            cleanup_ssl_context(&context);
        } else {
            printf("Failed to connect\\n");
        }
        printf("---\\n");
    }
    
    return 0;
}
