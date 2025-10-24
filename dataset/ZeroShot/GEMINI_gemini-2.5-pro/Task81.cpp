#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

// Helper to manage OpenSSL BIO pointers with std::unique_ptr
struct BIO_deleter {
    void operator()(BIO* bio) const { BIO_free_all(bio); }
};
using unique_BIO = std::unique_ptr<BIO, BIO_deleter>;

// Helper to manage OpenSSL SSL_CTX pointers with std::unique_ptr
struct SSL_CTX_deleter {
    void operator()(SSL_CTX* ctx) const { SSL_CTX_free(ctx); }
};
using unique_SSL_CTX = std::unique_ptr<SSL_CTX, SSL_CTX_deleter>;

// Helper to manage OpenSSL SSL pointers with std::unique_ptr
struct SSL_deleter {
    void operator()(SSL* ssl) const { SSL_free(ssl); }
};
using unique_SSL = std::unique_ptr<SSL, SSL_deleter>;

// Helper to manage OpenSSL X509 pointers with std::unique_ptr
struct X509_deleter {
    void operator()(X509* cert) const { X509_free(cert); }
};
using unique_X509 = std::unique_ptr<X509, X509_deleter>;


class OpenSSLInitializer {
public:
    OpenSSLInitializer() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
    }
    ~OpenSSLInitializer() {
        ERR_free_strings();
        EVP_cleanup();
    }
};

/**
 * Connects to a server, retrieves its certificate, and checks its SHA-256 hash.
 * @param hostname The server hostname.
 * @param expectedHash The expected SHA-256 hash in hex format.
 * @param port The server port.
 * @return True if the certificate hash matches, false otherwise.
 */
bool checkCertificateHash(const std::string& hostname, const std::string& expectedHash, int port = 443) {
    unique_SSL_CTX ctx(SSL_CTX_new(TLS_client_method()), SSL_CTX_deleter());
    if (!ctx) {
        std::cerr << "Error: Failed to create SSL_CTX" << std::endl;
        return false;
    }

    std::string connect_str = hostname + ":" + std::to_string(port);
    unique_BIO bio(BIO_new_ssl_connect(ctx.get()), BIO_deleter());
    if (!bio) {
        std::cerr << "Error: Failed to create BIO" << std::endl;
        return false;
    }

    BIO_set_conn_hostname(bio.get(), connect_str.c_str());

    SSL* ssl = nullptr;
    BIO_get_ssl(bio.get(), &ssl);
    if (!ssl) {
        std::cerr << "Error: Failed to get SSL pointer" << std::endl;
        return false;
    }

    // Enable SNI
    SSL_set_tlsext_host_name(ssl, hostname.c_str());

    if (BIO_do_connect(bio.get()) <= 0) {
        std::cerr << "Error: Failed to connect to " << hostname << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    unique_X509 cert(SSL_get_peer_certificate(ssl), X509_deleter());
    if (!cert) {
        std::cerr << "Error: No peer certificate received from " << hostname << std::endl;
        return false;
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    if (X509_digest(cert.get(), EVP_sha256(), hash, &hash_len) == 0) {
        std::cerr << "Error: Failed to compute certificate digest" << std::endl;
        return false;
    }

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    std::string actualHash = ss.str();
    
    std::string lowerExpectedHash = expectedHash;
    std::transform(lowerExpectedHash.begin(), lowerExpectedHash.end(), lowerExpectedHash.begin(), ::tolower);

    return actualHash == lowerExpectedHash;
}

int main() {
    // This object initializes OpenSSL upon creation and cleans it up upon destruction.
    OpenSSLInitializer ssl_initializer;

    // NOTE: Certificate hashes change when certificates are renewed. 
    // These test cases use a hash for example.com that was valid at the time of writing.
    // It may need to be updated. You can get the current hash with:
    // openssl s_client -connect example.com:443 -showcerts < /dev/null 2>/dev/null | openssl x509 -outform DER | openssl dgst -sha256 -hex
    //
    // COMPILE with: g++ your_file_name.cpp -o your_executable -lssl -lcrypto
    
    std::string exampleComHost = "example.com";
    // This hash is for the example.com certificate and may change over time.
    std::string correctExampleComHash = "3469e34a6058e5e959725f463385623089d7b901e0a9d8329b35e69e061517a9";
    std::string incorrectHash = "0000000000000000000000000000000000000000000000000000000000000000";

    // Test Case 1: Correct host and correct hash
    std::cout << "Test Case 1 (Correct): " << (checkCertificateHash(exampleComHost, correctExampleComHash) ? "Passed" : "Failed") << std::endl;

    // Test Case 2: Correct host and incorrect hash
    std::cout << "Test Case 2 (Incorrect Hash): " << (!checkCertificateHash(exampleComHost, incorrectHash) ? "Passed" : "Failed") << std::endl;

    // Test Case 3: Different host with the first host's hash
    std::cout << "Test Case 3 (Mismatched Host): " << (!checkCertificateHash("google.com", correctExampleComHash) ? "Passed" : "Failed") << std::endl;
    
    // Test Case 4: Non-existent host
    std::cout << "Test Case 4 (Invalid Host): " << (!checkCertificateHash("non-existent-domain-12345.com", correctExampleComHash) ? "Passed" : "Failed") << std::endl;

    // Test Case 5: Case-insensitive hash check
    std::string upperCaseHash = "3469E34A6058E5E959725F463385623089D7B901E0A9D8329B35E69E061517A9";
    std::cout << "Test Case 5 (Case-Insensitive): " << (checkCertificateHash(exampleComHost, upperCaseHash) ? "Passed" : "Failed") << std::endl;
    
    return 0;
}