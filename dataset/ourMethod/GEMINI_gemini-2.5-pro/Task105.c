/**
 * To compile this code:
 * gcc -o task105_secure_tls task105_secure_tls.c -lssl -lcrypto
 *
 * This program demonstrates how to establish a secure SSL/TLS connection
 * to a remote server using OpenSSL. It enforces TLS 1.2 or higher,
 * verifies the server's certificate against the system's default trust
 * store, and performs strict hostname verification.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

// Function to print OpenSSL error messages to stderr
void print_ssl_errors(void) {
    ERR_print_errors_fp(stderr);
}

/**
 * Creates a TCP socket and connects to the specified hostname and port.
 *
 * @param hostname The hostname of the server.
 * @param port The port number as a string.
 * @return The connected socket file descriptor, or -1 on failure.
 */
int create_socket_and_connect(const char *hostname, const char *port) {
    struct addrinfo hints;
    struct addrinfo *result = NULL, *rp = NULL;
    int sfd = -1;
    int ret;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(hostname, port, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        }

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break; // Success
        }

        close(sfd);
        sfd = -1;
    }

    freeaddrinfo(result);

    if (sfd == -1) {
        fprintf(stderr, "Could not connect to %s:%s\n", hostname, port);
    }

    return sfd;
}

/**
 * Establishes a secure TLS connection, sends a basic HTTP GET request,
 * and reads the response.
 *
 * @param hostname The hostname of the server to connect to.
 * @param port The port number as a string (e.g., "443").
 * @return 0 on success, -1 on failure.
 */
int secure_connect_and_request(const char *hostname, const char *port) {
    int status = -1;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int server_fd = -1;

    // 1. Initialize OpenSSL context
    // The method TLS_client_method() is a general-purpose one for all TLS/SSL versions.
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Failed to create SSL_CTX\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 2. Set minimum protocol version to TLS 1.2 for security
    // This prevents downgrade attacks to weaker protocols.
    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) != 1) {
        fprintf(stderr, "Failed to set minimum TLS protocol version\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 3. Load default system trust store
    // This is crucial for authenticating the server's certificate.
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "Failed to load default verify paths\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 4. Create and connect the underlying TCP socket
    server_fd = create_socket_and_connect(hostname, port);
    if (server_fd < 0) {
        goto cleanup;
    }

    // 5. Create SSL object and associate it with the socket
    ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Failed to create SSL object\n");
        print_ssl_errors();
        goto cleanup;
    }

    if (SSL_set_fd(ssl, server_fd) != 1) {
        fprintf(stderr, "Failed to set file descriptor on SSL object\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 6. Enable Server Name Indication (SNI)
    if (SSL_set_tlsext_host_name(ssl, hostname) != 1) {
        fprintf(stderr, "Failed to set SNI hostname\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 7. Enable strict hostname verification
    // This checks if the hostname in the certificate matches the one we are connecting to.
    if (SSL_set1_host(ssl, hostname) != 1) {
        fprintf(stderr, "Failed to set hostname for verification\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 8. Perform the TLS handshake
    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "SSL/TLS handshake failed with %s\n", hostname);
        print_ssl_errors();
        goto cleanup;
    }

    // 9. Verify the server certificate result
    long verify_result = SSL_get_verify_result(ssl);
    if (verify_result != X509_V_OK) {
        fprintf(stderr, "Certificate verification failed for %s: %s\n",
                hostname, X509_verify_cert_error_string(verify_result));
        goto cleanup;
    }

    printf("Successfully established TLS connection to %s:%s\n", hostname, port);
    printf("Using cipher: %s\n", SSL_get_cipher(ssl));

    // 10. Send an HTTP GET request
    char request[1024];
    int req_len = snprintf(request, sizeof(request),
                           "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
                           hostname);
    if (req_len < 0 || (size_t)req_len >= sizeof(request)) {
        fprintf(stderr, "Failed to create HTTP request\n");
        goto cleanup;
    }

    if (SSL_write(ssl, request, req_len) <= 0) {
        fprintf(stderr, "Failed to write to SSL connection\n");
        print_ssl_errors();
        goto cleanup;
    }

    // 11. Read the server's response
    char buffer[4096];
    int bytes_read;
    printf("--- Server Response ---\n");
    while ((bytes_read = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    printf("\n--- End of Response ---\n");

    if (bytes_read < 0) {
        int ssl_error = SSL_get_error(ssl, bytes_read);
        if (ssl_error != SSL_ERROR_ZERO_RETURN) { // Zero return is a clean closure
            fprintf(stderr, "SSL_read failed\n");
            print_ssl_errors();
            goto cleanup;
        }
    }
    
    status = 0; // Success

cleanup:
    if (ssl) {
        // Perform a clean shutdown.
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    if (ctx) {
        SSL_CTX_free(ctx);
    }
    return status;
}

int main(void) {
    // OpenSSL 1.1.0+ handles initialization automatically.
    // OPENSSL_init_ssl() is still safe to call.
    OPENSSL_init_ssl(0, NULL);

    const char *test_hosts[][2] = {
        {"www.google.com", "443"},
        {"www.openssl.org", "443"},
        {"expired.badssl.com", "443"},     // Expected to fail (expired cert)
        {"wrong.host.badssl.com", "443"},  // Expected to fail (hostname mismatch)
        {"self-signed.badssl.com", "443"}  // Expected to fail (untrusted root)
    };

    int num_tests = sizeof(test_hosts) / sizeof(test_hosts[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char *host = test_hosts[i][0];
        const char *port = test_hosts[i][1];
        printf("====================================================\n");
        printf("Attempting to connect to: %s:%s\n", host, port);
        printf("----------------------------------------------------\n");
        int result = secure_connect_and_request(host, port);
        if (result == 0) {
            printf("\n[SUCCESS] Connection to %s was successful.\n", host);
        } else {
            printf("\n[FAILURE] Connection to %s failed as expected for bad configurations or due to an error.\n", host);
        }
        printf("====================================================\n\n");
    }

    return 0;
}