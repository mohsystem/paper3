/*
 * Compile with: gcc -o task80_c task80.c -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// A structure to hold the socket and SSL object
typedef struct {
    int socket_fd;
    SSL* ssl;
} SslConnection;

// Function to initialize the OpenSSL library
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Function to clean up OpenSSL
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates an SSL client socket using IPv6 and TCP.
 *
 * @param hostname The hostname to connect to.
 * @param port The port to connect to.
 * @return An SslConnection struct with the socket and SSL object. On failure, socket_fd is -1 and ssl is NULL.
 */
SslConnection create_ipv6_ssl_client_socket(const char* hostname, int port) {
    SslConnection conn = {-1, NULL};
    SSL_CTX *ctx = NULL;

    // Create SSL context
    const SSL_METHOD *method = TLS_client_method();
    if (!method) {
        fprintf(stderr, "Error: Unable to get TLS client method.\n");
        return conn;
    }
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        fprintf(stderr, "Error: Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        return conn;
    }

    // Get address info for IPv6
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    if (getaddrinfo(hostname, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Error: getaddrinfo failed for host %s\n", hostname);
        SSL_CTX_free(ctx);
        return conn;
    }

    // Iterate through results and try to connect
    struct addrinfo* p;
    for (p = res; p != NULL; p = p->ai_next) {
        conn.socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (conn.socket_fd < 0) {
            continue;
        }

        if (connect(conn.socket_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break; // Successfully connected
        }

        close(conn.socket_fd);
        conn.socket_fd = -1;
    }

    freeaddrinfo(res);

    if (conn.socket_fd < 0) {
        fprintf(stderr, "Error: Failed to connect to any IPv6 address for %s\n", hostname);
        SSL_CTX_free(ctx);
        return conn;
    }

    // Create SSL object and attach it to the socket
    conn.ssl = SSL_new(ctx);
    SSL_set_fd(conn.ssl, conn.socket_fd);

    // Perform SSL/TLS handshake
    if (SSL_connect(conn.ssl) <= 0) {
        fprintf(stderr, "Error: SSL connection failed.\n");
        ERR_print_errors_fp(stderr);
        SSL_free(conn.ssl);
        close(conn.socket_fd);
        SSL_CTX_free(ctx);
        conn.socket_fd = -1;
        conn.ssl = NULL;
        return conn;
    }

    SSL_CTX_free(ctx); // Context is no longer needed
    return conn;
}

void close_connection(SslConnection conn) {
    if (conn.ssl) {
        SSL_shutdown(conn.ssl);
        SSL_free(conn.ssl);
    }
    if (conn.socket_fd >= 0) {
        close(conn.socket_fd);
    }
}

int main() {
    init_openssl();

    const char* hosts[] = {"google.com", "facebook.com", "ipv6.google.com", "wikipedia.org", "example.com"};
    int num_hosts = sizeof(hosts) / sizeof(hosts[0]);
    int port = 443;

    for (int i = 0; i < num_hosts; ++i) {
        printf("--- Testing connection to %s:%d ---\n", hosts[i], port);
        SslConnection conn = create_ipv6_ssl_client_socket(hosts[i], port);

        if (conn.ssl) {
            printf("Successfully connected to %s over IPv6.\n", hosts[i]);
            printf("Cipher: %s\n", SSL_get_cipher(conn.ssl));
            close_connection(conn);
            printf("Connection closed.\n");
        } else {
            printf("Failed to connect to %s over IPv6.\n", hosts[i]);
        }
        printf("\n");
    }

    cleanup_openssl();
    return 0;
}