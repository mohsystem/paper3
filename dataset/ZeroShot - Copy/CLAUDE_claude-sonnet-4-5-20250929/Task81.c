
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

void bytes_to_hex(const unsigned char* bytes, size_t len, char* output) {
    for (size_t i = 0; i < len; i++) {
        sprintf(output + (i * 2), "%02x", bytes[i]);
    }
    output[len * 2] = '\\0';
}

void normalize_hash(const char* hash, char* output) {
    int j = 0;
    for (int i = 0; hash[i] != '\\0'; i++) {
        if (hash[i] != ':' && hash[i] != ' ') {
            output[j++] = tolower(hash[i]);
        }
    }
    output[j] = '\\0';
}

int check_certificate_hash(const char* certificate_pem, const char* known_hash, const char* algorithm) {
    BIO* bio = BIO_new_mem_buf(certificate_pem, -1);
    if (!bio) return 0;
    
    X509* cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    BIO_free(bio);
    
    if (!cert) return 0;
    
    unsigned char* der = NULL;
    int der_len = i2d_X509(cert, &der);
    
    if (der_len < 0) {
        X509_free(cert);
        return 0;
    }
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    
    const EVP_MD* md;
    if (strcmp(algorithm, "SHA-256") == 0 || strcmp(algorithm, "SHA256") == 0 || strcmp(algorithm, "sha256") == 0) {
        md = EVP_sha256();
    } else if (strcmp(algorithm, "SHA-1") == 0 || strcmp(algorithm, "SHA1") == 0 || strcmp(algorithm, "sha1") == 0) {
        md = EVP_sha1();
    } else if (strcmp(algorithm, "MD5") == 0 || strcmp(algorithm, "md5") == 0) {
        md = EVP_md5();
    } else {
        OPENSSL_free(der);
        X509_free(cert);
        return 0;
    }
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, der, der_len);
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    OPENSSL_free(der);
    X509_free(cert);
    
    char cert_hash[EVP_MAX_MD_SIZE * 2 + 1];
    bytes_to_hex(hash, hash_len, cert_hash);
    
    char normalized_known_hash[256];
    normalize_hash(known_hash, normalized_known_hash);
    
    return strcmp(cert_hash, normalized_known_hash) == 0;
}

int main() {
    const char* test_cert = 
        "-----BEGIN CERTIFICATE-----\\n"
        "MIICLDCCAdKgAwIBAgIBADAKBggqhkjOPQQDAjB9MQswCQYDVQQGEwJ1czELMAkG\\n"
        "A1UECAwCQ0ExCzAJBgNVBAcMAlNGMQ8wDQYDVQQKDAZKb3lFbnQxEDAOBgNVBAsM\\n"
        "B05vZGUtY2ExFDASBgNVBAMMC2V4YW1wbGUuY29tMRswGQYJKoZIhvcNAQkBFgxh\\n"
        "QGV4YW1wbGUuY29tMB4XDTIxMDEwMTAwMDAwMFoXDTMxMDEwMTAwMDAwMFowfTEL\\n"
        "MAkGA1UEBhMCdXMxCzAJBgNVBAgMAkNBMQswCQYDVQQHDAJTRjEPMA0GA1UECgwG\\n"
        "Sm95RW50MRAwDgYDVQQLDAdOb2RlLWNhMRQwEgYDVQQDDAtleGFtcGxlLmNvbTEb\\n"
        "MBkGCSqGSIb3DQEJARYMYUBleGFtcGxlLmNvbTBZMBMGByqGSM49AgEGCCqGSM49\\n"
        "AwEHA0IABEg7eqR4yMGb2N8rTOi3OKrPtGLLzqVPnzqLjLwrPmLqLnVChdPMbZHL\\n"
        "LyZuMN0wPgwGNxQqPVLfvAp9fkqFfqajUDBOMB0GA1UdDgQWBBQ7WxJ8rYxNQvJm\\n"
        "lzWVhFWrfVWnlDAfBgNVHSMEGDAWgBQ7WxJ8rYxNQvJmlzWVhFWrfVWnlDAMBgNV\\n"
        "HRMEBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIQDLWlPGzNCPKqBvbLDPuLOdRVjJ\\n"
        "kIrGqTI6+xB3L2x8hAIgCHMzH8KwHgfTLxP9gFmqFYhpUkGJvVYMqfnCMZJmLrk=\\n"
        "-----END CERTIFICATE-----";
    
    // Test Case 1
    const char* hash1 = "8f43288ad272f3103b6fb1428485ea3014c0bcf547a2f1d4e8e1e2c6e5f7d8c9";
    printf("Test 1 - Valid SHA-256: %d\\n", check_certificate_hash(test_cert, hash1, "SHA-256"));
    
    // Test Case 2
    const char* hash2 = "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0";
    printf("Test 2 - Valid SHA-1: %d\\n", check_certificate_hash(test_cert, hash2, "SHA-1"));
    
    // Test Case 3
    const char* hash3 = "0000000000000000000000000000000000000000000000000000000000000000";
    printf("Test 3 - Invalid hash: %d\\n", check_certificate_hash(test_cert, hash3, "SHA-256"));
    
    // Test Case 4
    const char* hash4 = "8f:43:28:8a:d2:72:f3:10:3b:6f:b1:42:84:85:ea:30:14:c0:bc:f5:47:a2:f1:d4:e8:e1:e2:c6:e5:f7:d8:c9";
    printf("Test 4 - Hash with colons: %d\\n", check_certificate_hash(test_cert, hash4, "SHA-256"));
    
    // Test Case 5
    printf("Test 5 - Invalid cert: %d\\n", check_certificate_hash("invalid", hash1, "SHA-256"));
    
    return 0;
}
