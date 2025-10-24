
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

SSL* createSecureConnection(const char* host, int port) {
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        return NULL;
    }
    
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

char* sendHttpsRequest(const char* host, int port, const char* request) {
    SSL* ssl = createSecureConnection(host, port);
    if (!ssl) {
        return strdup("Error: Connection failed");
    }
    
    char fullRequest[2048];
    snprintf(fullRequest, sizeof(fullRequest), "%s\\r\\nHost: %s\\r\\nConnection: close\\r\\n\\r\\n", 
             request, host);
    
    SSL_write(ssl, fullRequest, strlen(fullRequest));
    
    char* response = malloc(10001);
    char buffer[4096];
    int bytes;
    int totalBytes = 0;
    response[0] = '\\0';
    
    while ((bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0 && totalBytes < 10000) {
        buffer[bytes] = '\\0';
        strncat(response, buffer, 10000 - totalBytes);
        totalBytes += bytes;
    }
    
    int sockfd = SSL_get_fd(ssl);
    SSL_free(ssl);
    close(sockfd);
    
    return response;
}

char* getConnectionInfo(const char* host, int port) {
    SSL* ssl = createSecureConnection(host, port);
    if (!ssl) {
        return strdup("Error: Connection failed");
    }
    
    char* info = malloc(512);
    snprintf(info, 512, "Protocol: %s\\nCipher Suite: %s\\n", 
             SSL_get_version(ssl), SSL_get_cipher(ssl));
    
    int sockfd = SSL_get_fd(ssl);
    SSL_free(ssl);
    close(sockfd);
    
    return info;
}

int verifyConnection(const char* host, int port) {
    SSL* ssl = createSecureConnection(host, port);
    if (!ssl) {
        return 0;
    }
    
    int sockfd = SSL_get_fd(ssl);
    SSL_free(ssl);
    close(sockfd);
    return 1;
}

int main() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    printf("Test Case 1: Verify secure connection to google.com:443\\n");
    int result1 = verifyConnection("google.com", 443);
    printf("Connection verified: %s\\n", result1 ? "true" : "false");
    
    printf("\\nTest Case 2: Get connection info for google.com:443\\n");
    char* info = getConnectionInfo("google.com", 443);
    printf("%s\\n", info);
    free(info);
    
    printf("\\nTest Case 3: Send HTTPS request to example.com:443\\n");
    char* response = sendHttpsRequest("example.com", 443, "GET / HTTP/1.1");
    printf("Response received (first 200 chars): %.200s\\n", response);
    free(response);
    
    printf("\\nTest Case 4: Verify connection to httpbin.org:443\\n");
    int result4 = verifyConnection("httpbin.org", 443);
    printf("Connection verified: %s\\n", result4 ? "true" : "false");
    
    printf("\\nTest Case 5: Test invalid connection\\n");
    int result5 = verifyConnection("invalid-host-12345.com", 443);
    printf("Connection verified: %s\\n", result5 ? "true" : "false");
    
    EVP_cleanup();
    ERR_free_strings();
    
    return 0;
}
