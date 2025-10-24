
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>

// RAII wrapper for SSL_CTX
class SSLContextWrapper {
private:
    SSL_CTX* ctx;
public:
    explicit SSLContextWrapper(const SSL_METHOD* method) {
        ctx = SSL_CTX_new(method);
        if (ctx) {
            // Force TLS 1.2+ to prevent downgrade attacks (CWE-327)
            SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
            // Enable certificate verification (CWE-295, CWE-297)
            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
            SSL_CTX_set_default_verify_paths(ctx);
        }
    }
    ~SSLContextWrapper() {
        if (ctx) {
            SSL_CTX_free(ctx);
        }
    }
    SSL_CTX* get() { return ctx; }
    SSLContextWrapper(const SSLContextWrapper&) = delete;
    SSLContextWrapper& operator=(const SSLContextWrapper&) = delete;
};

// RAII wrapper for SSL
class SSLWrapper {
private:
    SSL* ssl;
public:
    explicit SSLWrapper(SSL_CTX* ctx) {
        ssl = ctx ? SSL_new(ctx) : nullptr;
    }
    ~SSLWrapper() {
        if (ssl) {
            SSL_free(ssl);
        }
    }
    SSL* get() { return ssl; }
    SSLWrapper(const SSLWrapper&) = delete;
    SSLWrapper& operator=(const SSLWrapper&) = delete;
};

// RAII wrapper for BIO
class BIOWrapper {
private:
    BIO* bio;
public:
    explicit BIOWrapper(BIO* b) : bio(b) {}
    ~BIOWrapper() {
        if (bio) {
            BIO_free_all(bio);
        }
    }
    BIO* get() { return bio; }
    BIOWrapper(const BIOWrapper&) = delete;
    BIOWrapper& operator=(const BIOWrapper&) = delete;
};

/**
 * Retrieves the SSL certificate from a remote host.
 * Enforces TLS 1.2+ and proper certificate validation.
 * 
 * @param hostname The hostname to connect to
 * @param port The port to connect on
 * @param cert_der Output vector to store DER-encoded certificate
 * @return true on success, false on error
 */
bool getCertificateFromHost(const std::string& hostname, int port, std::vector<unsigned char>& cert_der) {
    // Input validation: hostname must be non-empty (CWE-20)
    if (hostname.empty()) {
        std::cerr << "Error: Invalid hostname" << std::endl;
        return false;
    }
    
    // Input validation: port must be in valid range (CWE-20)
    if (port < 1 || port > 65535) {
        std::cerr << "Error: Invalid port number" << std::endl;
        return false;
    }
    
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // Create SSL context with TLS method
    SSLContextWrapper ctx_wrapper(TLS_client_method());
    if (!ctx_wrapper.get()) {
        std::cerr << "Error: Failed to create SSL context" << std::endl;
        return false;
    }
    
    // Create BIO connection
    std::string conn_str = hostname + ":" + std::to_string(port);
    BIOWrapper bio_wrapper(BIO_new_ssl_connect(ctx_wrapper.get()));
    if (!bio_wrapper.get()) {
        std::cerr << "Error: Failed to create BIO" << std::endl;
        return false;
    }
    
    BIO_set_conn_hostname(bio_wrapper.get(), conn_str.c_str());
    
    // Get SSL pointer from BIO
    SSL* ssl = nullptr;
    BIO_get_ssl(bio_wrapper.get(), &ssl);
    if (!ssl) {
        std::cerr << "Error: Failed to get SSL from BIO" << std::endl;
        return false;
    }
    
    // Enable hostname verification (CWE-297)
    SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!SSL_set1_host(ssl, hostname.c_str())) {
        std::cerr << "Error: Failed to set hostname for verification" << std::endl;
        return false;
    }
    
    // Connect
    if (BIO_do_connect(bio_wrapper.get()) <= 0) {
        std::cerr << "Error: Connection failed" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    
    // Perform SSL handshake
    if (BIO_do_handshake(bio_wrapper.get()) <= 0) {
        std::cerr << "Error: SSL handshake failed" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    
    // Verify certificate
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        std::cerr << "Error: Certificate verification failed" << std::endl;
        return false;
    }
    
    // Get certificate
    X509* cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        std::cerr << "Error: No certificate received" << std::endl;
        return false;
    }
    
    // Convert certificate to DER format
    int der_len = i2d_X509(cert, nullptr);
    if (der_len < 0) {
        X509_free(cert);
        std::cerr << "Error: Failed to get certificate length" << std::endl;
        return false;
    }
    
    cert_der.resize(static_cast<size_t>(der_len));
    unsigned char* der_ptr = cert_der.data();
    if (i2d_X509(cert, &der_ptr) < 0) {
        X509_free(cert);
        std::cerr << "Error: Failed to encode certificate" << std::endl;
        return false;
    }
    
    X509_free(cert);
    return true;
}

/**
 * Computes the hash of a certificate using a strong cryptographic algorithm.
 * Only allows SHA-256 and SHA-512.
 * 
 * @param cert_der The DER-encoded certificate bytes
 * @param algorithm Hash algorithm to use ("sha256" or "sha512")
 * @param hash_hex Output string for hex-encoded hash
 * @return true on success, false on error
 */
bool computeCertificateHash(const std::vector<unsigned char>& cert_der, 
                            const std::string& algorithm,
                            std::string& hash_hex) {
    // Input validation: certificate data must be non-empty (CWE-20)
    if (cert_der.empty()) {
        std::cerr << "Error: Invalid certificate data" << std::endl;
        return false;
    }
    
    // Input validation: only allow secure hash algorithms (CWE-327)
    const EVP_MD* md = nullptr;
    if (algorithm == "sha256") {
        md = EVP_sha256();
    } else if (algorithm == "sha512") {
        md = EVP_sha512();
    } else {
        std::cerr << "Error: Unsupported hash algorithm. Use: sha256 or sha512" << std::endl;
        return false;
    }
    
    // Create hash context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        std::cerr << "Error: Failed to create hash context" << std::endl;
        return false;
    }
    
    bool success = false;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    
    do {
        if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
            std::cerr << "Error: Failed to initialize hash" << std::endl;
            break;
        }
        
        if (EVP_DigestUpdate(mdctx, cert_der.data(), cert_der.size()) != 1) {
            std::cerr << "Error: Failed to update hash" << std::endl;
            break;
        }
        
        if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
            std::cerr << "Error: Failed to finalize hash" << std::endl;
            break;
        }
        
        // Convert to hex string
        hash_hex.clear();
        hash_hex.reserve(hash_len * 2);
        for (unsigned int i = 0; i < hash_len; ++i) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", hash[i]);
            hash_hex += buf;
        }
        
        success = true;
    } while (false);
    
    EVP_MD_CTX_free(mdctx);
    return success;
}

/**
 * Constant-time string comparison to prevent timing attacks.
 * 
 * @param a First string
 * @param b Second string
 * @return true if strings are equal, false otherwise
 */
bool constantTimeCompare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    
    return result == 0;
}

/**
 * Verifies that a remote host's SSL certificate matches a known hash.\n * Uses constant-time comparison to prevent timing attacks.\n * \n * @param hostname The hostname to verify\n * @param known_hash The expected certificate hash (hex string)\n * @param port The port to connect on\n * @param algorithm Hash algorithm to use\n * @return true if certificate matches, false otherwise\n */\nbool verifyCertificateHash(const std::string& hostname, const std::string& known_hash,\n                          int port, const std::string& algorithm) {\n    // Input validation: hostname must be non-empty (CWE-20)\n    if (hostname.empty()) {\n        std::cerr << "Error: Invalid hostname" << std::endl;\n        return false;\n    }\n    \n    // Input validation: known hash must be non-empty (CWE-20)\n    if (known_hash.empty()) {\n        std::cerr << "Error: Invalid known hash" << std::endl;\n        return false;\n    }\n    \n    // Validate known hash is valid hex\n    for (char c : known_hash) {\n        if (!std::isxdigit(static_cast<unsigned char>(c))) {\n            std::cerr << "Error: Known hash must be a valid hex string" << std::endl;\n            return false;\n        }\n    }\n    \n    // Validate expected hash length based on algorithm\n    size_t expected_length = 0;\n    if (algorithm == "sha256") {\n        expected_length = 64;\n    } else if (algorithm == "sha512") {\n        expected_length = 128;\n    }\n    \n    if (known_hash.length() != expected_length) {\n        std::cerr << "Error: Hash length mismatch for " << algorithm << std::endl;\n        return false;\n    }\n    \n    // Retrieve certificate from remote host\n    std::vector<unsigned char> cert_der;\n    if (!getCertificateFromHost(hostname, port, cert_der)) {\n        return false;\n    }\n    \n    // Compute hash of retrieved certificate\n    std::string computed_hash;\n    if (!computeCertificateHash(cert_der, algorithm, computed_hash)) {\n        return false;\n    }\n    \n    // Convert both hashes to lowercase for comparison\n    std::string known_lower = known_hash;\n    std::string computed_lower = computed_hash;\n    std::transform(known_lower.begin(), known_lower.end(), known_lower.begin(),\n                  [](unsigned char c) { return std::tolower(c); });\n    std::transform(computed_lower.begin(), computed_lower.end(), computed_lower.begin(),\n                  [](unsigned char c) { return std::tolower(c); });\n    \n    // Use constant-time comparison to prevent timing attacks\n    if (constantTimeCompare(known_lower, computed_lower)) {\n        return true;\n    } else {\n        // Do not reveal which part of the hash mismatched (fail closed)\n        std::cerr << "Error: Certificate hash does not match known hash" << std::endl;\n        return false;\n    }\n}\n\nint main() {\n    std::cout << "=== SSL Certificate Hash Verification Tests ===" << std::endl << std::endl;\n    \n    // Test Case 1: Retrieve and hash example.com certificate\n    std::cout << "Test 1: Retrieve and hash example.com certificate" << std::endl;\n    try {\n        std::vector<unsigned char> cert;\n        if (getCertificateFromHost("example.com", 443, cert)) {\n            std::string hash_val;\n            if (computeCertificateHash(cert, "sha256", hash_val)) {\n                std::cout << "Success: SHA-256 hash = " << hash_val << std::endl << std::endl;\n            } else {\n                std::cout << "Failed: Could not compute hash" << std::endl << std::endl;\n            }\n        } else {\n            std::cout << "Failed: Could not retrieve certificate" << std::endl << std::endl;\n        }\n    } catch (const std::exception& e) {\n        std::cout << "Test 1 exception: " << e.what() << std::endl << std::endl;\n    }\n    \n    // Test Case 2: Verify with incorrect hash (should fail)\n    std::cout << "Test 2: Verify example.com with incorrect hash (should fail)" << std::endl;\n    std::string incorrect_hash(64, '0');\n    bool result = verifyCertificateHash("example.com", incorrect_hash, 443, "sha256");\n    std::cout << "Result: " << (!result ? "PASS - Correctly rejected" : "FAIL - Incorrectly accepted") << std::endl << std::endl;\n    \n    // Test Case 3: Invalid hostname\n    std::cout << "Test 3: Invalid hostname (should fail gracefully)" << std::endl;\n    result = verifyCertificateHash("", std::string(64, '0'), 443, "sha256");\n    std::cout << "Result: " << (!result ? "PASS - Handled error" : "FAIL") << std::endl << std::endl;\n    \n    // Test Case 4: Invalid port\n    std::cout << "Test 4: Invalid port number (should fail)" << std::endl;\n    std::vector<unsigned char> cert;\n    bool cert_result = getCertificateFromHost("example.com", 99999, cert);\n    std::cout << "Result: " << (!cert_result ? "PASS - Handled error" : "FAIL") << std::endl << std::endl;\n    \n    // Test Case 5: Invalid hash algorithm\n    std::cout << "Test 5: Unsupported hash algorithm (should fail)" << std::endl;\n    std::vector<unsigned char> test_data = {'t', 'e', 's', 't'};
    std::string hash_result;
    bool hash_success = computeCertificateHash(test_data, "md5", hash_result);
    std::cout << "Result: " << (!hash_success ? "PASS - Rejected insecure algorithm" : "FAIL") << std::endl << std::endl;
    
    std::cout << "=== All tests completed ===" << std::endl;
    
    return 0;
}
