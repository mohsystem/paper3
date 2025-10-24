/*
 * To compile and run this C++ code, you need to have OpenSSL installed.
 *
 * Compilation command:
 * g++ -o Task81_cpp Task81.cpp -lssl -lcrypto
 *
 * Execution command:
 * ./Task81_cpp
 */
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

class Task81 {
public:
    /**
     * Converts a byte array to a hexadecimal string.
     */
    static std::string bytesToHex(const unsigned char* bytes, size_t len) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; ++i) {
            ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
        }
        return ss.str();
    }

    /**
     * Checks if a server's SSL certificate matches a known SHA-256 hash.
     * @param hostname The server hostname.
     * @param port The server port.
     * @param knownHash The expected SHA-256 hash as a hex string.
     * @return True if the certificate hash matches, false otherwise.
     */
    static bool checkCertificateHash(const std::string& hostname, int port, const std::string& knownHash) {
        // For a real application, OpenSSL should be initialized once.
        // For this self-contained example, we do it here.
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            return false;
        }

        BIO* bio = BIO_new_ssl_connect(ctx);
        if (!bio) {
            SSL_CTX_free(ctx);
            return false;
        }

        SSL* ssl;
        BIO_get_ssl(bio, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

        // Set hostname for Server Name Indication (SNI)
        SSL_set_tlsext_host_name(ssl, hostname.c_str());

        std::string host_port = hostname + ":" + std::to_string(port);
        BIO_set_conn_hostname(bio, host_port.c_str());

        // BIO_do_connect will perform the connection and handshake
        if (BIO_do_connect(bio) <= 0) {
            BIO_free_all(bio);
            SSL_CTX_free(ctx);
            return false;
        }

        X509* cert = SSL_get_peer_certificate(ssl);
        bool result = false;

        if (cert) {
            unsigned char hash_buf[SHA256_DIGEST_LENGTH];
            unsigned char* cert_der = nullptr;
            // Get the certificate in DER format
            int len = i2d_X509(cert, &cert_der);

            if (len > 0) {
                // Calculate the SHA-256 hash
                SHA256(cert_der, len, hash_buf);
                std::string calculatedHash = bytesToHex(hash_buf, SHA256_DIGEST_LENGTH);
                
                // Case-insensitive comparison
                if (calculatedHash.size() == knownHash.size()) {
                    result = std::equal(calculatedHash.begin(), calculatedHash.end(),
                                        knownHash.begin(),
                                        [](char a, char b) {
                                            return tolower(a) == tolower(b);
                                        });
                }
                OPENSSL_free(cert_der);
            }
            X509_free(cert);
        }

        // Cleanup
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        // These are no-ops in modern OpenSSL (1.1.0+) but are kept for compatibility.
        EVP_cleanup();
        ERR_free_strings();

        return result;
    }
};

int main() {
    std::string correctHash = "42a3219451add031a017243a05c45b882d9d1502425576a8049e7b39920194eb";
    std::string incorrectHash = "0000000000000000000000000000000000000000000000000000000000000000";

    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Success
    bool test1 = Task81::checkCertificateHash("sha256.badssl.com", 443, correctHash);
    std::cout << "Test 1 (Success): " << (test1 ? "Passed" : "Failed") << std::endl;

    // Test Case 2: Failure - Wrong Hash
    bool test2 = Task81::checkCertificateHash("sha256.badssl.com", 443, incorrectHash);
    std::cout << "Test 2 (Failure - Wrong Hash): " << (!test2 ? "Passed" : "Failed") << std::endl;

    // Test Case 3: Failure - Wrong Host
    bool test3 = Task81::checkCertificateHash("google.com", 443, correctHash);
    std::cout << "Test 3 (Failure - Wrong Host): " << (!test3 ? "Passed" : "Failed") << std::endl;

    // Test Case 4: Failure - Non-existent Host
    bool test4 = Task81::checkCertificateHash("nonexistent.domain.local", 443, correctHash);
    std::cout << "Test 4 (Failure - Non-existent Host): " << (!test4 ? "Passed" : "Failed") << std::endl;

    // Test Case 5: Failure - No SSL
    bool test5 = Task81::checkCertificateHash("neverssl.com", 443, correctHash);
    std::cout << "Test 5 (Failure - No SSL): " << (!test5 ? "Passed" : "Failed") << std::endl;
    
    return 0;
}