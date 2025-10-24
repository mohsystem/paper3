#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

// Helper to print the OpenSSL error stack
void print_openssl_errors() {
    unsigned long err_code;
    while ((err_code = ERR_get_error())) {
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        fprintf(stderr, "OpenSSL error: %s\n", err_buf);
    }
}

// Structure to hold both the SSL object and socket file descriptor
typedef struct {
    SSL* ssl;
    int socket_fd;
} SslConnection;

// Creates an IPv6 SSL connection. Returns a struct with ssl=NULL on failure.
// The caller is responsible for calling cleanup_connection on the returned struct.
SslConnection create_ssl_ipv6_socket(const char* host, int port) {
    SslConnection conn = { .ssl = NULL, .socket_fd = -1 };
    SSL_CTX* ctx = NULL;
    struct addrinfo hints;
    struct addrinfo *result = NULL, *rp = NULL;
    int sfd = -1;

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Port out of valid range (1-65535)\n");
        return conn;
    }

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Failed to create SSL_CTX\n");
        print_openssl_errors();
        goto cleanup;
    }

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "Failed to load default CA certificates\n");
        print_openssl_errors();
        goto cleanup;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    if (getaddrinfo(host, port_str, &hints, &result) != 0) {
        fprintf(stderr, "getaddrinfo failed for host: %s\n", host);
        goto cleanup;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0) break; // Success
        close(sfd);
        sfd = -1;
    }

    if (rp == NULL) {
        fprintf(stderr, "Failed to connect to any resolved IPv6 address for %s\n", host);
        goto cleanup;
    }

    conn.ssl = SSL_new(ctx);
    if (!conn.ssl) {
        print_openssl_errors();
        goto cleanup;
    }
    
    conn.socket_fd = sfd;
    sfd = -1; // Ownership transferred

    SSL_set_fd(conn.ssl, conn.socket_fd);
    SSL_set_tlsext_host_name(conn.ssl, host); // Set SNI
    
    // Set hostname for verification
    X509_VERIFY_PARAM *param = SSL_get0_param(conn.ssl);
    X509_VERIFY_PARAM_set1_host(param, host, 0);
    SSL_set_verify(conn.ssl, SSL_VERIFY_PEER, NULL);

    if (SSL_connect(conn.ssl) != 1) {
        fprintf(stderr, "SSL handshake failed with %s\n", host);
        print_openssl_errors();
        goto cleanup;
    }

    // Success path: clean up only temporary resources
    freeaddrinfo(result);
    SSL_CTX_free(ctx);
    return conn;

cleanup: // Error path: clean up everything
    if (conn.ssl) SSL_free(conn.ssl);
    if (conn.socket_fd != -1) close(conn.socket_fd);
    if (sfd != -1) close(sfd);
    if (result) freeaddrinfo(result);
    if (ctx) SSL_CTX_free(ctx);
    conn.ssl = NULL;
    conn.socket_fd = -1;
    return conn;
}

void cleanup_connection(SslConnection* conn) {
    if (conn) {
        if (conn->ssl) {
            SSL_shutdown(conn->ssl);
            SSL_free(conn->ssl);
            conn->ssl = NULL;
        }
        if (conn->socket_fd != -1) {
            close(conn->socket_fd);
            conn->socket_fd = -1;
        }
    }
}

int main() {
    OPENSSL_init_ssl(0, NULL);
    
    const char* test_hosts[] = {"google.com", "ietf.org", "wikipedia.org", "thishostdoesnotexist12345.com", "google.com"};
    int test_ports[] = {443, 443, 443, 443, 99999};
    int num_tests = sizeof(test_hosts) / sizeof(test_hosts[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* host = test_hosts[i];
        int port = test_ports[i];
        printf("--- Test Case %d: Connecting to %s:%d over IPv6 ---\n", i + 1, host, port);

        SslConnection conn = create_ssl_ipv6_socket(host, port);
        if (conn.ssl) {
            printf("Successfully connected to %s:%d\n", host, port);
            printf("SSL Protocol: %s\n", SSL_get_version(conn.ssl));
            printf("Cipher Suite: %s\n", SSL_get_cipher_name(conn.ssl));
            
            char request[512];
            snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
            SSL_write(conn.ssl, request, (int)strlen(request));

            char buffer[1024];
            int bytes = SSL_read(conn.ssl, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                char* first_line = strtok(buffer, "\r\n");
                if (first_line) printf("Server response (first line): %s\n", first_line);
            }
            cleanup_connection(&conn);
        } else {
            fprintf(stderr, "Failed to connect to %s:%d\n", host, port);
        }
        printf("\n");
    }

    return 0;
}