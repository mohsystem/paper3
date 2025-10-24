
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

char* base64_decode(const char* input, size_t* output_length) {
    BIO *bio, *b64;
    size_t input_length = strlen(input);
    char *buffer = (char*)malloc(input_length);
    
    bio = BIO_new_mem_buf(input, input_length);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    *output_length = BIO_read(bio, buffer, input_length);
    BIO_free_all(bio);
    
    return buffer;
}

void bytes_to_hex(const unsigned char* data, size_t len, char* output) {
    for (size_t i = 0; i < len; i++) {
        sprintf(output + (i * 2), "%02x", data[i]);
    }
    output[len * 2] = '\\0';
}

void normalize_hash(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i] != '\\0'; i++) {
        if (input[i] != ':' && input[i] != ' ') {
            output[j++] = tolower(input[i]);
        }
    }
    output[j] = '\\0';
}

int checkCertificateHash(const char* certificate_pem, const char* known_hash, const char* algorithm) {
    // Find certificate content
    const char* start_marker = "-----BEGIN CERTIFICATE-----";
    const char* end_marker = "-----END CERTIFICATE-----";
    
    const char* start = strstr(certificate_pem, start_marker);
    const char* end = strstr(certificate_pem, end_marker);
    
    if (start == NULL || end == NULL) {
        return 0;
    }
    
    start += strlen(start_marker);
    size_t content_len = end - start;
    
    // Extract and clean certificate content
    char* cert_content = (char*)malloc(content_len + 1);
    int j = 0;
    for (size_t i = 0; i < content_len; i++) {
        if (!isspace(start[i])) {
            cert_content[j++] = start[i];
        }
    }
    cert_content[j] = '\\0';
    
    // Decode base64
    size_t decoded_len;
    char* cert_bytes = base64_decode(cert_content, &decoded_len);
    free(cert_content);
    
    if (cert_bytes == NULL || decoded_len == 0) {
        return 0;
    }
    
    // Calculate hash
    char calculated_hash[256];
    
    if (strcmp(algorithm, "SHA-256") == 0 || strcmp(algorithm, "sha256") == 0) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)cert_bytes, decoded_len, hash);
        bytes_to_hex(hash, SHA256_DIGEST_LENGTH, calculated_hash);
    } else if (strcmp(algorithm, "SHA-1") == 0 || strcmp(algorithm, "sha1") == 0) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)cert_bytes, decoded_len, hash);
        bytes_to_hex(hash, SHA_DIGEST_LENGTH, calculated_hash);
    } else if (strcmp(algorithm, "MD5") == 0 || strcmp(algorithm, "md5") == 0) {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)cert_bytes, decoded_len, hash);
        bytes_to_hex(hash, MD5_DIGEST_LENGTH, calculated_hash);
    } else {
        free(cert_bytes);
        return 0;
    }
    
    free(cert_bytes);
    
    // Normalize and compare
    char normalized_known[256];
    normalize_hash(known_hash, normalized_known);
    
    return strcmp(calculated_hash, normalized_known) == 0;
}

int main() {
    const char* test_cert = 
        "-----BEGIN CERTIFICATE-----\\n"
        "MIICWzCCAcSgAwIBAgIJAL7xG3YqkqWGMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV\\n"
        "BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX\\n"
        "aWRnaXRzIFB0eSBMdGQwHhcNMjEwMTAxMDAwMDAwWhcNMjIwMTAxMDAwMDAwWjBF\\n"
        "MQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50\\n"
        "ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKB\\n"
        "gQDHc0BcF8y6tN3jKBvXvL7VdqOYWlYwXMdGpLvXgNF5h6m6R7X9J4qH8o4F3k9Q\\n"
        "y6z8K7dF9e3pQ6mL4x2X7fG5j8K9L3m4x5N6p7qQ8R3j4K9L2m5x6N7q8R3j5K9L\\n"
        "3m5x7N8q9R4j6K0L4m6x8N0q0R5j7K1L5m7x9N1q1R6j8K2L6m8xQIDAQABo1Aw\\n"
        "TjAdBgNVHQ4EFgQU8h3j9K0L4m6x8N0q0R5j7K1L5m8wHwYDVR0jBBgwFoAU8h3j\\n"
        "9K0L4m6x8N0q0R5j7K1L5m8wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOB\\n"
        "gQBZ3j9K1L5m7x9N1q1R6j8K2L6m8x0N2q2R7j9K3L7m9xN3q3R8jK4L8mxN4q4R\\n"
        "9jK5L9mx0N5q5R0jK6LmxN6q6R1jK7LmxN7q7R2jK8LmxN8q8R3jK9Lmx\\n"
        "-----END CERTIFICATE-----";
    
    printf("Test 1 - Valid SHA-256: %d\\n", 
           checkCertificateHash(test_cert, "a1b2c3d4e5f6", "SHA-256"));
    
    printf("Test 2 - Invalid hash: %d\\n", 
           checkCertificateHash(test_cert, "0000000000000000", "SHA-256"));
    
    printf("Test 3 - SHA-1: %d\\n", 
           checkCertificateHash(test_cert, "abcdef123456", "SHA-1"));
    
    printf("Test 4 - MD5: %d\\n", 
           checkCertificateHash(test_cert, "fedcba654321", "MD5"));
    
    printf("Test 5 - Invalid cert: %d\\n", 
           checkCertificateHash("-----BEGIN CERTIFICATE-----\\nINVALID\\n-----END CERTIFICATE-----",
                               "a1b2c3d4e5f6", "SHA-256"));
    
    return 0;
}
