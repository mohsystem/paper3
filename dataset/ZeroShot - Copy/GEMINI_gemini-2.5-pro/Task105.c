#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// Socket headers (for POSIX systems)
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// Note: To compile this code, you need to link against OpenSSL libraries.
// Example command: gcc Task105.c -o Task105 -lssl -lcrypto

void init_openssl() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}

void cleanup_openssl() {
    EVP_cleanup();
    ERR_free_strings();
}

int create_tcp_socket(const char *host, int port) {
    struct addrinfo hints, *res, *p;
    int sock_fd = -1;
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        fprintf(stderr, "getaddrinfo error for host: %s\n", host);
        return -1;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) continue;
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == 0) break; // Success
        close(sock_fd);
        sock_fd = -1;
    }
    freeaddrinfo(res);
    return sock_fd;
}

bool establishSecureConnection(const char* host, int port) {
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int sock_fd = -1;
    bool success = false;

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Error creating SSL_CTX.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }
    
    // Set secure options: disable insecure protocols
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "Error loading default CA certificates.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    sock_fd = create_tcp_socket(host, port);
    if (sock_fd < 0) {
        fprintf(stderr, "Failed to create or connect socket for %s:%d\n", host, port);
        goto cleanup;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Error creating SSL object.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }
    SSL_set_fd(ssl, sock_fd);

    // Enable SNI (Server Name Indication)
    if (SSL_set_tlsext_host_name(ssl, host) != 1) {
        fprintf(stderr, "Error setting SNI hostname.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }
    
    // Enable hostname verification
    X509_VERIFY_PARAM *param = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!X509_VERIFY_PARAM_set1_host(param, host, 0)) {
        fprintf(stderr, "Error setting hostname for verification.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }
    SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);

    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "SSL Handshake Failed for %s:%d.\n", host, port);
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    long verify_result = SSL_get_verify_result(ssl);
    if (verify_result != X509_V_OK) {
        fprintf(stderr, "Certificate verification failed for %s:%d. Error: %s\n",
                host, port, X509_verify_cert_error_string(verify_result));
        goto cleanup;
    }

    printf("Successfully connected to %s:%d\n", host, port);
    printf("SSL Session Info:\n");
    printf("\tProtocol: %s\n", SSL_get_version(ssl));
    printf("\tCipher Suite: %s\n", SSL_get_cipher(ssl));

    success = true;

cleanup:
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock_fd >= 0) {
        close(sock_fd);
    }
    if (ctx) {
        SSL_CTX_free(ctx);
    }
    return success;
}

int main() {
    init_openssl();

    const char* hosts[] = {"google.com", "github.com", "expired.badssl.com", "wrong.host.badssl.com", "nonexistent-domain-xyz123.com"};
    int port = 443;
    int num_tests = sizeof(hosts) / sizeof(hosts[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Test Case %d: Connecting to %s:%d ---\n", i + 1, hosts[i], port);
        bool success = establishSecureConnection(hosts[i], port);
        printf("Connection status: %s\n", success ? "SUCCESS" : "FAILURE");
        printf("----------------------------------------------------\n");
    }

    cleanup_openssl();
    return 0;
}