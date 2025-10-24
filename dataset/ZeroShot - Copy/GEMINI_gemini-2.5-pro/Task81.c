#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

void initialize_openssl() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}

void cleanup_openssl() {
    ERR_free_strings();
    EVP_cleanup();
}

/**
 * Connects to a server, retrieves its certificate, and checks its SHA-256 hash.
 * @param hostname The server hostname.
 * @param expected_hash The expected SHA-256 hash in hex format.
 * @param port The server port.
 * @return True if the certificate hash matches, false otherwise.
 */
bool checkCertificateHash(const char* hostname, const char* expected_hash, int port) {
    bool result = false;
    SSL_CTX* ctx = NULL;
    BIO* bio = NULL;
    SSL* ssl = NULL;
    X509* cert = NULL;

    char connect_str[256];
    snprintf(connect_str, sizeof(connect_str), "%s:%d", hostname, port);

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create SSL_CTX\n");
        goto cleanup;
    }

    bio = BIO_new_ssl_connect(ctx);
    if (!bio) {
        fprintf(stderr, "Error: Failed to create BIO\n");
        goto cleanup;
    }

    BIO_set_conn_hostname(bio, connect_str);

    BIO_get_ssl(bio, &ssl);
    if (!ssl) {
        fprintf(stderr, "Error: Failed to get SSL pointer\n");
        goto cleanup;
    }

    // Enable SNI
    SSL_set_tlsext_host_name(ssl, hostname);

    if (BIO_do_connect(bio) <= 0) {
        fprintf(stderr, "Error: Failed to connect to %s\n", hostname);
        ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        fprintf(stderr, "Error: No peer certificate received from %s\n", hostname);
        goto cleanup;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (X509_digest(cert, EVP_sha256(), hash, &hash_len) == 0) {
        fprintf(stderr, "Error: Failed to compute certificate digest\n");
        goto cleanup;
    }

    char actual_hash_hex[2 * EVP_MAX_MD_SIZE + 1];
    for (unsigned int i = 0; i < hash_len; ++i) {
        sprintf(&actual_hash_hex[i * 2], "%02x", hash[i]);
    }
    actual_hash_hex[hash_len * 2] = '\0';
    
    if (strcasecmp(actual_hash_hex, expected_hash) == 0) {
        result = true;
    }

cleanup:
    if (cert) X509_free(cert);
    if (bio) BIO_free_all(bio); // BIO_new_ssl_connect requires BIO_free_all
    if (ctx) SSL_CTX_free(ctx);

    return result;
}

int main() {
    initialize_openssl();

    // NOTE: Certificate hashes change when certificates are renewed. 
    // These test cases use a hash for example.com that was valid at the time of writing.
    // It may need to be updated. You can get the current hash with:
    // openssl s_client -connect example.com:443 -showcerts < /dev/null 2>/dev/null | openssl x509 -outform DER | openssl dgst -sha256 -hex
    //
    // COMPILE with: gcc your_file_name.c -o your_executable -lssl -lcrypto
    
    const char* example_com_host = "example.com";
    // This hash is for the example.com certificate and may change over time.
    const char* correct_example_com_hash = "3469e34a6058e5e959725f463385623089d7b901e0a9d8329b35e69e061517a9";
    const char* incorrect_hash = "0000000000000000000000000000000000000000000000000000000000000000";

    // Test Case 1: Correct host and correct hash
    printf("Test Case 1 (Correct): %s\n", checkCertificateHash(example_com_host, correct_example_com_hash, 443) ? "Passed" : "Failed");

    // Test Case 2: Correct host and incorrect hash
    printf("Test Case 2 (Incorrect Hash): %s\n", !checkCertificateHash(example_com_host, incorrect_hash, 443) ? "Passed" : "Failed");

    // Test Case 3: Different host with the first host's hash
    printf("Test Case 3 (Mismatched Host): %s\n", !checkCertificateHash("google.com", correct_example_com_hash, 443) ? "Passed" : "Failed");
    
    // Test Case 4: Non-existent host
    printf("Test Case 4 (Invalid Host): %s\n", !checkCertificateHash("non-existent-domain-12345.com", correct_example_com_hash, 443) ? "Passed" : "Failed");
    
    // Test Case 5: Case-insensitive hash check
    const char* upper_case_hash = "3469E34A6058E5E959725F463385623089D7B901E0A9D8329B35E69E061517A9";
    printf("Test Case 5 (Case-Insensitive): %s\n", checkCertificateHash(example_com_host, upper_case_hash, 443) ? "Passed" : "Failed");
    
    cleanup_openssl();
    return 0;
}