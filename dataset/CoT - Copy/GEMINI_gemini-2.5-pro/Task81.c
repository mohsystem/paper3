/*
 * To compile and run this C code, you need to have OpenSSL installed.
 *
 * Compilation command:
 * gcc -o Task81_c Task81.c -lssl -lcrypto
 *
 * Execution command:
 * ./Task81_c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

/**
 * Converts a byte array to a hexadecimal string.
 * The caller must ensure the hex_str buffer is large enough (len * 2 + 1).
 */
void bytesToHex(const unsigned char* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

/**
 * Case-insensitive string comparison.
 * Returns 0 if strings are equal.
 */
int strcicmp(char const *a, char const *b) {
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a) {
            return d;
        }
    }
}

/**
 * Checks if a server's SSL certificate matches a known SHA-256 hash.
 * @param hostname The server hostname.
 * @param port The server port.
 * @param knownHash The expected SHA-256 hash as a hex string.
 * @return 1 if the certificate hash matches, 0 otherwise.
 */
int checkCertificateHash(const char* hostname, int port, const char* knownHash) {
    SSL_CTX* ctx = NULL;
    BIO* bio = NULL;
    SSL* ssl = NULL;
    X509* cert = NULL;
    int result = 0; // 0 for false

    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) goto cleanup;

    bio = BIO_new_ssl_connect(ctx);
    if (!bio) goto cleanup;

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    
    // Set hostname for Server Name Indication (SNI)
    SSL_set_tlsext_host_name(ssl, hostname);

    char host_port[256];
    snprintf(host_port, sizeof(host_port), "%s:%d", hostname, port);
    BIO_set_conn_hostname(bio, host_port);

    if (BIO_do_connect(bio) <= 0) {
        goto cleanup;
    }

    cert = SSL_get_peer_certificate(ssl);
    if (cert) {
        unsigned char hash_buf[SHA256_DIGEST_LENGTH];
        unsigned char* cert_der = NULL;
        int len = i2d_X509(cert, &cert_der);

        if (len > 0) {
            SHA256(cert_der, len, hash_buf);
            char calculatedHash[SHA256_DIGEST_LENGTH * 2 + 1];
            bytesToHex(hash_buf, SHA256_DIGEST_LENGTH, calculatedHash);
            
            if (strcicmp(calculatedHash, knownHash) == 0) {
                result = 1; // 1 for true
            }
            OPENSSL_free(cert_der);
        }
    }

cleanup:
    if (cert) X509_free(cert);
    if (bio) BIO_free_all(bio);
    if (ctx) SSL_CTX_free(ctx);
    // No-ops in modern OpenSSL but kept for compatibility.
    EVP_cleanup();
    ERR_free_strings();
    
    return result;
}

int main() {
    const char* correctHash = "42a3219451add031a017243a05c45b882d9d1502425576a8049e7b39920194eb";
    const char* incorrectHash = "0000000000000000000000000000000000000000000000000000000000000000";

    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Success
    int test1 = checkCertificateHash("sha256.badssl.com", 443, correctHash);
    printf("Test 1 (Success): %s\n", test1 ? "Passed" : "Failed");

    // Test Case 2: Failure - Wrong Hash
    int test2 = checkCertificateHash("sha256.badssl.com", 443, incorrectHash);
    printf("Test 2 (Failure - Wrong Hash): %s\n", !test2 ? "Passed" : "Failed");

    // Test Case 3: Failure - Wrong Host
    int test3 = checkCertificateHash("google.com", 443, correctHash);
    printf("Test 3 (Failure - Wrong Host): %s\n", !test3 ? "Passed" : "Failed");

    // Test Case 4: Failure - Non-existent Host
    int test4 = checkCertificateHash("nonexistent.domain.local", 443, correctHash);
    printf("Test 4 (Failure - Non-existent Host): %s\n", !test4 ? "Passed" : "Failed");

    // Test Case 5: Failure - No SSL
    int test5 = checkCertificateHash("neverssl.com", 443, correctHash);
    printf("Test 5 (Failure - No SSL): %s\n", !test5 ? "Passed" : "Failed");

    return 0;
}