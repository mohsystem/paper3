/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against libssl and libcrypto.
 * Example compilation command on Linux/macOS:
 * gcc Task80.c -o Task80 -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Global setup for OpenSSL, called once.
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Global cleanup for OpenSSL, called once.
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates a secure SSL client socket using IPv6 and connects to the specified host and port.
 *
 * @param hostname The hostname to connect to.
 * @param port The port number to connect to.
 * @return A pointer to the SSL structure on success, or NULL on failure.
 *         The caller is responsible for freeing the SSL and SSL_CTX objects and closing the socket.
 */
SSL* create_ipv6_ssl_socket(const char* hostname, int port) {
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int sock_fd = -1;

    // 1. Create SSL Context
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Error: Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    // Load default trust store for certificate validation
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "Error: Failed to load default verify paths.\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }

    // 2. Resolve hostname to IPv6 address and connect
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    if (getaddrinfo(hostname, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Error: Cannot resolve IPv6 address for %s\n", hostname);
        SSL_CTX_free(ctx);
        return NULL;
    }

    printf("Resolved IPv6 address for %s\n", hostname);
    sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd < 0) {
        perror("socket");
        freeaddrinfo(res);
        SSL_CTX_free(ctx);
        return NULL;
    }

    if (connect(sock_fd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "Error: Cannot connect to %s:%d\n", hostname, port);
        perror("connect");
        close(sock_fd);
        freeaddrinfo(res);
        SSL_CTX_free(ctx);
        return NULL;
    }
    freeaddrinfo(res);

    // 3. Create SSL structure and perform handshake
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);

    // Set SNI (Server Name Indication) - crucial for many modern servers
    SSL_set_tlsext_host_name(ssl, hostname);

    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "Error: SSL handshake failed with %s\n", hostname);
        ERR_print_errors_fp(stderr);
        SSL_free(ssl); // Frees SSL and closes the socket fd
        SSL_CTX_free(ctx);
        return NULL;
    }

    printf("SSL handshake successful with %s:%d\n", hostname, port);
    SSL_CTX_free(ctx); // Context can be freed after SSL object creation
    return ssl;
}

int main() {
    init_openssl();

    // --- Test Cases ---
    const char* hosts[] = {"google.com", "facebook.com", "wikipedia.org", "nonexistent.veryunlikelydomain.com", "google.com"};
    int ports[] = {443, 443, 443, 443, 444};
    int num_tests = sizeof(hosts) / sizeof(hosts[0]);

    for (int i = 0; i < num_tests; i++) {
        const char* host = hosts[i];
        int port = ports[i];

        printf("\n--- Test Case %d: Connecting to %s:%d ---\n", i + 1, host, port);

        SSL* ssl = create_ipv6_ssl_socket(host, port);

        if (ssl) {
            printf("Successfully connected to %s:%d\n", host, port);
            
            // Perform a simple HTTP GET to verify connection
            char request[512];
            snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
            SSL_write(ssl, request, strlen(request));

            char buffer[1024] = {0};
            int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                char* first_line_end = strstr(buffer, "\r\n");
                if (first_line_end) {
                    *first_line_end = '\0';
                }
                printf("Response from server: %s\n", buffer);
            }
            
            // Cleanup
            int sock_fd = SSL_get_fd(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sock_fd);
        } else {
            printf("Failed to connect to %s:%d\n", host, port);
        }
    }

    cleanup_openssl();
    return 0;
}