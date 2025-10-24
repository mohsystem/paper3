/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile on Linux/macOS:
 * gcc your_file_name.c -o your_executable -lssl -lcrypto
 *
 * On Windows, you will need to install OpenSSL and configure your compiler
 * to link against the necessary libraries (e.g., libssl.lib, libcrypto.lib).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// Socket-related headers (POSIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

/**
 * Initializes the OpenSSL library. Should be called once per application.
 */
void init_openssl() {
    // These are thread-safe and can be called multiple times.
    // In OpenSSL 1.1.0+, this is done automatically.
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

/**
 * Cleans up the OpenSSL library.
 */
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates a TCP connection to the specified host and port.
 * @return The socket file descriptor, or -1 on failure.
 */
int create_tcp_socket(const char* host, int port) {
    struct hostent* he;
    if ((he = gethostbyname(host)) == NULL) {
        fprintf(stderr, "ERROR: Could not resolve hostname: %s\n", host);
        return -1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("ERROR: Could not create socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr*)he->h_addr_list[0]);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Could not connect to host");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

/**
 * Establishes a secure SSL/TLS connection to a remote server.
 *
 * @param host The hostname of the server.
 * @param port The port number of the server.
 * @return true if the connection was successfully established, false otherwise.
 */
bool establishTlsConnection(const char* host, int port) {
    SSL_CTX* ctx = NULL;
    SSL* ssl = NULL;
    int sock_fd = -1;
    bool success = false;

    printf("Attempting to connect to %s:%d over TLS...\n", host, port);

    // Create SSL context. TLS_client_method() is the modern, preferred method.
    ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == NULL) {
        fprintf(stderr, "ERROR: Could not create SSL context.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    // Load default trusted CA certificates from the system's standard locations.
    // This is crucial for server certificate verification.
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "ERROR: Could not load default CA certificates.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    sock_fd = create_tcp_socket(host, port);
    if (sock_fd == -1) {
        goto cleanup;
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        fprintf(stderr, "ERROR: Could not create SSL structure.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    SSL_set_fd(ssl, sock_fd);

    // Set SNI and enable hostname verification
    if (SSL_set_tlsext_host_name(ssl, host) != 1) {
        fprintf(stderr, "ERROR: Could not set SNI hostname.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }
    X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set1_host(param, host, 0);
    SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);

    // Perform TLS handshake
    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "ERROR: TLS handshake failed.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    // Verify certificate
    long verify_result = SSL_get_verify_result(ssl);
    if (verify_result != X509_V_OK) {
        fprintf(stderr, "ERROR: Certificate verification failed with code: %ld (%s)\n",
                verify_result, X509_verify_cert_error_string(verify_result));
        goto cleanup;
    }

    printf("TLS connection established successfully!\n");
    printf("Using protocol: %s\n", SSL_get_version(ssl));
    printf("Using cipher: %s\n", SSL_get_cipher(ssl));
    
    success = true;

cleanup:
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock_fd != -1) {
        close(sock_fd);
    }
    if (ctx) {
        SSL_CTX_free(ctx);
    }
    return success;
}

int main() {
    init_openssl();

    printf("--- Running SSL/TLS Connection Tests ---\n");

    // Test Case 1
    printf("\n[Test Case 1: www.google.com]\n");
    bool result1 = establishTlsConnection("www.google.com", 443);
    printf("Test 1 Result: %s\n", result1 ? "SUCCESS" : "FAILURE");

    // Test Case 2
    printf("\n[Test Case 2: www.github.com]\n");
    bool result2 = establishTlsConnection("www.github.com", 443);
    printf("Test 2 Result: %s\n", result2 ? "SUCCESS" : "FAILURE");

    // Test Case 3
    printf("\n[Test Case 3: expired.badssl.com]\n");
    bool result3 = establishTlsConnection("expired.badssl.com", 443);
    printf("Test 3 Result: %s\n", !result3 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)");

    // Test Case 4
    printf("\n[Test Case 4: wrong.host.badssl.com]\n");
    bool result4 = establishTlsConnection("wrong.host.badssl.com", 443);
    printf("Test 4 Result: %s\n", !result4 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)");

    // Test Case 5
    printf("\n[Test Case 5: non-existent-domain-for-testing-123.com]\n");
    bool result5 = establishTlsConnection("non-existent-domain-for-testing-123.com", 443);
    printf("Test 5 Result: %s\n", !result5 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)");

    printf("\n--- All tests completed. ---\n");

    cleanup_openssl();
    return 0;
}