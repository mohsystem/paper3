
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

SSL* establish_secure_connection(const char* host, int port) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        return NULL;
    }
    
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_cipher_list(ctx, "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:!aNULL:!MD5:!DSS");
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_default_verify_paths(ctx);
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    struct hostent* server = gethostbyname(host);
    if (!server) {
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);
    
    if (SSL_connect(ssl) <= 0) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    return ssl;
}

char* send_request(const char* host, int port, const char* request) {
    SSL* ssl = establish_secure_connection(host, port);
    if (!ssl) {
        return NULL;
    }
    
    SSL_write(ssl, request, strlen(request));
    
    char* response = (char*)malloc(10001);
    memset(response, 0, 10001);
    char buffer[4096];
    int bytes;
    int total = 0;
    
    while ((bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0 && total < 10000) {
        buffer[bytes] = '\\0';
        strcat(response, buffer);
        total += bytes;
        if (strstr(response, "\\r\\n\\r\\n")) {
            break;
        }
    }
    
    int sockfd = SSL_get_fd(ssl);
    SSL_free(ssl);
    close(sockfd);
    
    return response;
}

int main() {
    printf("SSL/TLS Secure Connection Test Cases:\\n\\n");
    
    // Test case 1
    printf("Test 1: Connecting to www.google.com:443\\n");
    char* response1 = send_request("www.google.com", 443, 
        "GET / HTTP/1.1\\r\\nHost: www.google.com\\r\\n\\r\\n");
    if (response1) {
        printf("Connection successful! Response length: %lu\\n", strlen(response1));
        free(response1);
    } else {
        printf("Test 1 failed\\n");
    }
    
    // Test case 2
    printf("\\nTest 2: Connecting to github.com:443\\n");
    char* response2 = send_request("github.com", 443, 
        "GET / HTTP/1.1\\r\\nHost: github.com\\r\\n\\r\\n");
    if (response2) {
        printf("Connection successful! Response length: %lu\\n", strlen(response2));
        free(response2);
    } else {
        printf("Test 2 failed\\n");
    }
    
    // Test case 3
    printf("\\nTest 3: Connecting to www.amazon.com:443\\n");
    char* response3 = send_request("www.amazon.com", 443, 
        "GET / HTTP/1.1\\r\\nHost: www.amazon.com\\r\\n\\r\\n");
    if (response3) {
        printf("Connection successful! Response length: %lu\\n", strlen(response3));
        free(response3);
    } else {
        printf("Test 3 failed\\n");
    }
    
    // Test case 4
    printf("\\nTest 4: Connecting to www.microsoft.com:443\\n");
    char* response4 = send_request("www.microsoft.com", 443, 
        "GET / HTTP/1.1\\r\\nHost: www.microsoft.com\\r\\n\\r\\n");
    if (response4) {
        printf("Connection successful! Response length: %lu\\n", strlen(response4));
        free(response4);
    } else {
        printf("Test 4 failed\\n");
    }
    
    // Test case 5
    printf("\\nTest 5: Connecting to invalid.host.example:443\\n");
    char* response5 = send_request("invalid.host.example", 443, 
        "GET / HTTP/1.1\\r\\nHost: invalid.host.example\\r\\n\\r\\n");
    if (response5) {
        printf("Connection successful! Response length: %lu\\n", strlen(response5));
        free(response5);
    } else {
        printf("Test 5 failed (expected)\\n");
    }
    
    return 0;
}
