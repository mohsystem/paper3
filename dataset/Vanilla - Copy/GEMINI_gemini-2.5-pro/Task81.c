#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define strcasecmp _stricmp
#else
#include <strings.h> // For strcasecmp
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

// Helper to convert raw bytes to a hex string
void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

bool checkCertificateHash(const char* host, int port, const char* known_hash) {
    SSL_CTX* ctx = NULL;
    BIO* bio = NULL;
    SSL* ssl = NULL;
    X509* cert = NULL;
    unsigned char* der_cert_buf = NULL;
    bool match = false;

    ctx = SSL_CTX_new(TLS_client_method());
    if (ctx == NULL) {
        //fprintf(stderr, "  Error: Unable to create SSL context.\n");
        goto cleanup;
    }

    bio = BIO_new_ssl_connect(ctx);
    if (bio == NULL) {
        //fprintf(stderr, "  Error: Unable to create BIO.\n");
        goto cleanup;
    }

    BIO_get_ssl(bio, &ssl);
    if (ssl == NULL) {
        //fprintf(stderr, "  Error: Unable to get SSL pointer.\n");
        goto cleanup;
    }
    
    // Set SNI (Server Name Indication), crucial for shared hosting
    SSL_set_tlsext_host_name(ssl, host);

    char host_port[256];
    snprintf(host_port, sizeof(host_port), "%s:%d", host, port);
    BIO_set_conn_hostname(bio, host_port);

    if (BIO_do_connect(bio) <= 0) {
        //fprintf(stderr, "  Error connecting to %s\n", host);
        //ERR_print_errors_fp(stderr);
        goto cleanup;
    }

    cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL) {
        fprintf(stderr, "  Error: Could not get server certificate.\n");
        goto cleanup;
    }

    int len = i2d_X509(cert, &der_cert_buf);
    if (len < 0) {
        fprintf(stderr, "  Error: Could not encode certificate to DER.\n");
        goto cleanup;
    }

    unsigned char hash_buf[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (EVP_Digest(der_cert_buf, len, hash_buf, &hash_len, EVP_sha256(), NULL) != 1) {
        fprintf(stderr, "  Error: Could not compute SHA-256 digest.\n");
        goto cleanup;
    }
    
    char calculated_hash[EVP_MAX_MD_SIZE * 2 + 1];
    bytes_to_hex(hash_buf, hash_len, calculated_hash);

    if (strcasecmp(calculated_hash, known_hash) == 0) {
        match = true;
    }

cleanup:
    if (der_cert_buf) OPENSSL_free(der_cert_buf);
    if (cert) X509_free(cert);
    if (bio) BIO_free_all(bio);
    if (ctx) SSL_CTX_free(ctx);

    return match;
}

int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif

    // NOTE: These hashes are time-sensitive and may need to be updated as
    // websites rotate their SSL certificates. The hashes provided are for the
    // *entire* DER-encoded certificate, not just the public key.
    // To compile on Linux: gcc your_file.c -o your_program -lssl -lcrypto
    // On Windows, you may need to configure include/library paths and link against
    // libssl.lib, libcrypto.lib, ws2_32.lib etc.

    struct TestCase { const char* host; const char* hash; bool expected; };
    struct TestCase testCases[] = {
        {"google.com", "19f7a72675b331481b21cb4e488135891e4f34614207963d722108709328246a", true},
        {"github.com", "346b45b239094d2b2746487e452a2656973e6f21789c6d5b035f5022e032230a", true},
        {"example.com", "this_is_an_incorrect_hash_string_for_testing_failure1234567890", false},
        {"self-signed.badssl.com", "987724f28012626e205a91438b417c8227b403f752496a71e16c905f884a4411", true},
        {"thishostdoesnotexist12345.com", "any_hash_will_do_since_host_is_unreachable", false}
    };
    int numTests = sizeof(testCases) / sizeof(struct TestCase);

    printf("--- Running C Test Cases ---\n");
    for (int i = 0; i < numTests; i++) {
        printf("Test %d: Checking host %s\n", i + 1, testCases[i].host);
        bool result = checkCertificateHash(testCases[i].host, 443, testCases[i].hash);
        printf("  Result: %s\n", result ? "true" : "false");
        printf("  Expected: %s\n", testCases[i].expected ? "true" : "false");
        printf("  Status: %s\n\n", (result == testCases[i].expected ? "PASS" : "FAIL"));
    }

    ERR_free_strings();
    EVP_cleanup();
    
    #ifdef _WIN32
    WSACleanup();
    #endif

    return 0;
}