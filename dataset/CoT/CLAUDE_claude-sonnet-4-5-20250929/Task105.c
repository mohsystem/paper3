
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define MAX_HOSTNAME_LEN 256

/**
 * Establishes a secure SSL/TLS connection with a remote server
 */
char* establish_secure_connection(const char* hostname, int port, const char* message) {
    static char result[BUFFER_SIZE * 2];
    memset(result, 0, sizeof(result));
    
    if (!hostname || strlen(hostname) == 0) {
        strncpy(result, "Error: Invalid hostname", sizeof(result) - 1);
        return result;
    }
    
    if (port < 1 || port > 65535) {
        strncpy(result, "Error: Invalid port number", sizeof(result) - 1);
        return result;
    }
    
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    int sock = -1;
    
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // Create SSL context
    const SSL_METHOD* method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        strncpy(result, "Error: Failed to create SSL context", sizeof(result) - 1);
        return result;
    }
    
    // Set minimum TLS version
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    
    // Load CA certificates
    if (!SSL_CTX_set_default_verify_paths(ctx)) {
        SSL_CTX_free(ctx);
        strncpy(result, "Error: Failed to load CA certificates", sizeof(result) - 1);
        return result;
    }
    
    // Enable certificate verification
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    
    // Set secure ciphers
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4");
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        SSL_CTX_free(ctx);
        strncpy(result, "Error: Failed to create socket", sizeof(result) - 1);
        return result;
    }
    
    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    // Resolve hostname
    struct hostent* host = gethostbyname(hostname);
    if (!host) {
        close(sock);
        SSL_CTX_free(ctx);
        strncpy(result, "Error: Failed to resolve hostname", sizeof(result) - 1);
        return result;
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
        strncpy(result, "Error: Failed to connect to server", sizeof(result) - 1);
        return result;
    }
    
    // Create SSL structure
    ssl = SSL_new(ctx);
    if (!ssl) {
        close(sock);
        SSL_CTX_free(ctx);
        strncpy(result, "Error: Failed to create SSL structure", sizeof(result) - 1);
        return result;
    }
    
    // Set hostname for SNI
    SSL_set_tlsext_host_name(ssl, hostname);
    
    // Attach socket
    SSL_set_fd(ssl, sock);
    
    // Perform handshake
    if (SSL_connect(ssl) <= 0) {
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        strncpy(result, "Error: SSL handshake failed", sizeof(result) - 1);
        return result;
    }
    
    // Verify certificate
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        SSL_free(ssl);
        close(sock);
        SSL_CTX_free(ctx);
        strncpy(result, "Error: Certificate verification failed", sizeof(result) - 1);
        return result;
    }
    
    // Build result string
    snprintf(result, sizeof(result), "Connection successful. Protocol: %s, Cipher: %s\\n",
             SSL_get_version(ssl), SSL_get_cipher(ssl));
    
    // Send message if provided
    if (message && strlen(message) > 0) {
        int written = SSL_write(ssl, message, strlen(message));
        if (written > 0) {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));
            int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                strncat(result, "Response: ", sizeof(result) - strlen(result) - 1);
                strncat(result, buffer, sizeof(result) - strlen(result) - 1);
            }
        }
    }
    
    // Cleanup
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    
    return result;
}

int main() {
    printf("=== SSL/TLS Secure Connection Test Cases ===\\n\\n");
    
    // Test Case 1: Valid connection
    printf("Test Case 1: Valid HTTPS connection\\n");
    char* result1 = establish_secure_connection("www.google.com", 443, 
                                                 "GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\n\\r\\n");
    printf("%.200s...\\n\\n", result1);
    
    // Test Case 2: Empty hostname
    printf("Test Case 2: Empty hostname\\n");
    char* result2 = establish_secure_connection("", 443, NULL);
    printf("%s\\n\\n", result2);
    
    // Test Case 3: Invalid port
    printf("Test Case 3: Invalid port\\n");
    char* result3 = establish_secure_connection("www.google.com", 99999, NULL);
    printf("%s\\n\\n", result3);
    
    // Test Case 4: Low port number
    printf("Test Case 4: Invalid low port\\n");
    char* result4 = establish_secure_connection("www.google.com", 0, NULL);
    printf("%s\\n\\n", result4);
    
    // Test Case 5: NULL hostname
    printf("Test Case 5: NULL hostname\\n");
    char* result5 = establish_secure_connection(NULL, 443, NULL);
    printf("%s\\n\\n", result5);
    
    return 0;
}
