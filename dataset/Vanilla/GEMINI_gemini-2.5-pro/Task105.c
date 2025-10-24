/*
 * COMPILE: gcc Task105.c -o task105_c -lssl -lcrypto
 * RUN: ./task105_c
 * NOTE: Requires OpenSSL development libraries (e.g., on Ubuntu/Debian: sudo apt-get install libssl-dev).
 * This code uses POSIX sockets and will not compile on Windows without changes (e.g., using Winsock).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// Networking headers
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

void init_openssl() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}

void cleanup_openssl() {
    EVP_cleanup();
}

int create_and_connect_socket(const char *hostname, int port) {
    int sock_fd = -1;
    struct addrinfo hints, *servinfo, *p;
    char port_str[6];
    
    snprintf(port_str, sizeof(port_str), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, port_str, &hints, &servinfo) != 0) {
        fprintf(stderr, "getaddrinfo failed for %s\n", hostname);
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd == -1) continue;
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) != -1) break;
        close(sock_fd);
        sock_fd = -1;
    }
    
    freeaddrinfo(servinfo);
    return sock_fd;
}

bool establishSSLConnection(const char* host, int port) {
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int server_fd = -1;
    bool success = false;
    
    printf("Attempting to connect to %s:%d...\n", host, port);

    const SSL_METHOD *method = TLS_client_method();
    if (method == NULL) {
        fprintf(stderr, "Unable to create SSL method.\n");
        goto cleanup;
    }

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        fprintf(stderr, "Unable to create SSL context.\n");
        goto cleanup;
    }

    server_fd = create_and_connect_socket(host, port);
    if (server_fd == -1) {
        fprintf(stderr, "Failed to create or connect socket for %s.\n", host);
        goto cleanup;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        fprintf(stderr, "Failed to create SSL structure.\n");
        goto cleanup;
    }
    
    SSL_set_fd(ssl, server_fd);
    SSL_set_tlsext_host_name(ssl, host); // Enable SNI

    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "SSL handshake failed for %s.\n", host);
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    printf("Successfully connected to %s:%d\n", host, port);
    printf("SSL Connection Info:\n");
    printf("\tProtocol: %s\n", SSL_get_version(ssl));
    printf("\tCipher: %s\n", SSL_get_cipher_name(ssl));
    
    success = true;

cleanup:
    if (ssl != NULL) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    if (ctx != NULL) {
        SSL_CTX_free(ctx);
    }
    printf("----------------------------------------\n");
    return success;
}

int main() {
    init_openssl();

    // 5 Test Cases
    establishSSLConnection("google.com", 443);
    establishSSLConnection("github.com", 443);
    establishSSLConnection("cloudflare.com", 443);
    establishSSLConnection("amazon.com", 443);
    establishSSLConnection("nonexistent-host-for-testing.com", 443); // Failure case
    
    cleanup_openssl();
    return 0;
}