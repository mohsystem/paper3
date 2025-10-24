/**
 * @file task.c
 * @brief Checks an SSL certificate against a known hash (certificate pinning).
 *
 * This program connects to a server over TLS, retrieves its leaf certificate,
 * and compares the SHA-256 hash of that certificate with a known, trusted hash.
 * It performs full certificate chain and hostname validation before checking the hash.
 *
 * Compile with: gcc -Wall -Wextra -o task task.c -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // For strcasecmp
#include <unistd.h>  // For close

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>

/**
 * @brief Checks if the SSL certificate of a remote server matches a known SHA-256 hash.
 * 
 * This function connects to the given hostname and port, performs a full TLS handshake
 * including certificate chain and hostname validation, and then compares the SHA-256
 * hash of the leaf certificate against a provided known hash.
 * 
 * @param hostname The hostname of the server to connect to.
 * @param port The port of the server to connect to.
 * @param known_hash_hex A string containing the expected 64-character hex-encoded SHA-256 hash.
 * @return 1 if the hash matches, 0 if it mismatches, -1 on any error (connection, verification, etc.).
 */
int check_cert_hash(const char *hostname, int port, const char *known_hash_hex);

int main() {
    // --- Test cases ---
    // Note: The "correct" hash will change over time as certificates are renewed.
    // This hash was valid for www.openssl.org at the time of writing.
    // You can get the current hash with:
    // openssl s_client -connect www.openssl.org:443 -servername www.openssl.org < /dev/null 2>/dev/null | openssl x509 -outform DER | openssl dgst -sha256
    const char *openssl_host = "www.openssl.org";
    const char *openssl_correct_hash = "061d3a95be837c355828484f33b1e5a40b93802773138b321151676d9818820c";
    
    printf("--- Running 5 Test Cases ---\n\n");

    // Test Case 1: Correct host and correct hash
    printf("Test Case 1: Connecting to %s with correct hash...\n", openssl_host);
    int result1 = check_cert_hash(openssl_host, 443, openssl_correct_hash);
    printf("Result 1: %s\n\n", (result1 == 1) ? "SUCCESS (Hash Matched)" : "FAILURE");

    // Test Case 2: Correct host but incorrect hash
    const char *wrong_hash = "0000000000000000000000000000000000000000000000000000000000000000";
    printf("Test Case 2: Connecting to %s with WRONG hash...\n", openssl_host);
    int result2 = check_cert_hash(openssl_host, 443, wrong_hash);
    printf("Result 2: %s\n\n", (result2 == 0) ? "SUCCESS (Hash Mismatch Correctly Detected)" : "FAILURE");

    // Test Case 3: Different host (google.com) with openssl.org's hash
    const char *google_host = "www.google.com";
    printf("Test Case 3: Connecting to %s with %s's hash...\n", google_host, openssl_host);
    int result3 = check_cert_hash(google_host, 443, openssl_correct_hash);
    printf("Result 3: %s\n\n", (result3 == 0) ? "SUCCESS (Hash Mismatch Correctly Detected)" : "FAILURE");

    // Test Case 4: Non-existent host
    const char *invalid_host = "thishostdoesnotexist.invalid";
    printf("Test Case 4: Connecting to non-existent host %s...\n", invalid_host);
    int result4 = check_cert_hash(invalid_host, 443, openssl_correct_hash);
    printf("Result 4: %s\n\n", (result4 == -1) ? "SUCCESS (Error Correctly Detected)" : "FAILURE");
    
    // Test Case 5: Host with an untrusted certificate (self-signed)
    const char *badssl_host = "self-signed.badssl.com";
    printf("Test Case 5: Connecting to %s (self-signed cert)...\n", badssl_host);
    printf("This should fail on certificate chain validation.\n");
    int result5 = check_cert_hash(badssl_host, 443, wrong_hash);
    printf("Result 5: %s\n\n", (result5 == -1) ? "SUCCESS (Verification Error Correctly Detected)" : "FAILURE");

    return 0;
}

int check_cert_hash(const char *hostname, int port, const char *known_hash_hex) {
    if (!hostname || port <= 0 || port > 65535 || !known_hash_hex || strlen(known_hash_hex) != 64) {
        fprintf(stderr, "Error: Invalid arguments provided.\n");
        return -1;
    }

    int result = -1; // -1:error, 0:mismatch, 1:match
    long verify_result;
    
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    X509 *cert = NULL;
    int sock_fd = -1;
    struct addrinfo hints, *servinfo = NULL, *p = NULL;
    
    OPENSSL_init_ssl(0, NULL);

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Error: Unable to create SSL context.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) != 1) {
        fprintf(stderr, "Error: Failed to set minimum TLS protocol version.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        fprintf(stderr, "Error: Failed to load default CA trust store.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, port_str, &hints, &servinfo) != 0) {
        perror("getaddrinfo");
        goto cleanup;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) != -1) {
            break; // Success
        }
        close(sock_fd);
        sock_fd = -1;
    }

    if (p == NULL) {
        fprintf(stderr, "Error: Failed to connect to host %s.\n", hostname);
        goto cleanup;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "Error: Unable to create SSL structure.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    SSL_set_fd(ssl, sock_fd);
    if (SSL_set_tlsext_hostname(ssl, hostname) != 1) {
        fprintf(stderr, "Error: Failed to set SNI hostname.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    X509_VERIFY_PARAM *param = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set1_host(param, hostname, 0);
    SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);

    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "Error: SSL handshake failed with %s.\n", hostname);
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }
    
    verify_result = SSL_get_verify_result(ssl);
    if (verify_result != X509_V_OK) {
        fprintf(stderr, "Error: Certificate chain verification failed: %s\n", 
                X509_verify_cert_error_string(verify_result));
        goto cleanup;
    }

    cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        fprintf(stderr, "Error: Could not get peer certificate from %s.\n", hostname);
        goto cleanup;
    }

    unsigned char calculated_hash[SHA256_DIGEST_LENGTH];
    unsigned char *der_cert_buf = NULL;
    int der_cert_len = i2d_X509(cert, &der_cert_buf);

    if (der_cert_len < 0) {
        fprintf(stderr, "Error: Failed to encode certificate to DER format.\n");
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    if (!SHA256(der_cert_buf, der_cert_len, calculated_hash)) {
        fprintf(stderr, "Error: SHA256 calculation failed.\n");
        ERR_print_errors_fp(stderr);
        OPENSSL_free(der_cert_buf);
        goto cleanup;
    }
    OPENSSL_free(der_cert_buf);

    char calculated_hash_hex[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(calculated_hash_hex + (i * 2), 3, "%02x", calculated_hash[i]);
    }
    
    if (strcasecmp(calculated_hash_hex, known_hash_hex) == 0) {
        result = 1; // Match
    } else {
        fprintf(stderr, "CERTIFICATE HASH MISMATCH!\n");
        fprintf(stderr, "  Expected: %s\n", known_hash_hex);
        fprintf(stderr, "  Actual  : %s\n", calculated_hash_hex);
        result = 0; // Mismatch
    }

cleanup:
    if (cert) X509_free(cert);
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock_fd != -1) close(sock_fd);
    if (servinfo) freeaddrinfo(servinfo);
    if (ctx) SSL_CTX_free(ctx);

    return result;
}