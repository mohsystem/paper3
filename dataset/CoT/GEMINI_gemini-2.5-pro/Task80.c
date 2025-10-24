/*
 * C SSL Client for IPv6
 * Note: This code requires the OpenSSL library.
 * To compile: gcc your_file.c -o your_app -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX/Linux headers for socket programming
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 * Initializes the OpenSSL library. Should be called once per application.
 */
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

/**
 * Cleans up the OpenSSL library resources.
 */
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates, connects, and performs the SSL handshake for an IPv6 client socket.
 *
 * @param host The hostname to connect to.
 * @param port The port number.
 * @return A pointer to the established SSL connection object.
 *         Returns NULL on failure. The caller is responsible for a full cleanup on success.
 *         Full cleanup procedure:
 *         1. int sock_fd = SSL_get_fd(ssl);
 *         2. SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
 *         3. SSL_shutdown(ssl); // Graceful shutdown
 *         4. SSL_free(ssl);
 *         5. close(sock_fd);
 *         6. SSL_CTX_free(ctx);
 */
SSL* create_ssl_ipv6_client_socket(const char* host, int port) {
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    int sock_fd = -1;
    struct addrinfo hints, *result, *rp;
    char port_str[6]; // max port is 65535

    // Create a new SSL context
    const SSL_METHOD* method = TLS_client_method();
    if (!method) {
        fprintf(stderr, "Error: Unable to create SSL/TLS client method.\n");
        return NULL;
    }
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        fprintf(stderr, "Error: Unable to create SSL context.\n");
        return NULL;
    }
    
    // Load default CA certificates for server verification.
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "Error: Failed to load default CA certificates.\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    // Resolve hostname to IPv6 address
    snprintf(port_str, sizeof(port_str), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port_str, &hints, &result) != 0) {
        perror("getaddrinfo");
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    // Create and connect socket
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock_fd == -1) continue;
        if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1) break; // Success
        close(sock_fd);
        sock_fd = -1;
    }
    freeaddrinfo(result);
    if (sock_fd == -1) {
        fprintf(stderr, "Error: Could not create or connect IPv6 socket for %s\n", host);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    // Create SSL object and associate with socket
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);
    
    // Set SNI and enable hostname verification.
    if (SSL_set1_host(ssl, host) != 1) {
        fprintf(stderr, "Error: SSL_set1_host (SNI) failed.\n");
        SSL_free(ssl); close(sock_fd); SSL_CTX_free(ctx);
        return NULL;
    }

    // Perform handshake
    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "Error: SSL/TLS handshake failed for host %s\n", host);
        ERR_print_errors_fp(stderr);
        SSL_free(ssl); close(sock_fd); SSL_CTX_free(ctx);
        return NULL;
    }

    // Verify certificate
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        fprintf(stderr, "Error: Certificate verification failed for %s\n", host);
        SSL_free(ssl); close(sock_fd); SSL_CTX_free(ctx);
        return NULL;
    }

    return ssl; // Success
}

int main() {
    init_openssl();
    
    const char* test_hosts[] = {
        "google.com",
        "www.facebook.com",
        "ipv6.google.com",
        "www.ietf.org",
        "badssl.com"
    };
    int num_hosts = sizeof(test_hosts) / sizeof(test_hosts[0]);
    int port = 443;

    for (int i = 0; i < num_hosts; ++i) {
        const char* host = test_hosts[i];
        printf("--- Testing connection to %s:%d ---\n", host, port);
        
        SSL* ssl = create_ssl_ipv6_client_socket(host, port);
        
        if (ssl) {
            printf("Successfully connected to %s\n", host);
            printf("SSL/TLS Protocol: %s\n", SSL_get_version(ssl));
            printf("Cipher Suite: %s\n", SSL_get_cipher(ssl));
            
            // Full cleanup
            int sock_fd = SSL_get_fd(ssl);
            SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sock_fd);
            SSL_CTX_free(ctx);
        } else {
            fprintf(stderr, "Connection test failed for %s\n", host);
        }
        printf("\n");
    }

    cleanup_openssl();
    return 0;
}