#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define strcasecmp _stricmp
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>

// Helper to convert raw bytes to a hex string
std::string bytesToHex(const unsigned char* bytes, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(bytes[i]);
    }
    return ss.str();
}

// Custom deleters for OpenSSL resources to be used with std::unique_ptr for RAII
struct BIO_deleter { void operator()(BIO* b) const { BIO_free_all(b); } };
struct X509_deleter { void operator()(X509* x) const { X509_free(x); } };
struct SSL_CTX_deleter { void operator()(SSL_CTX* c) const { SSL_CTX_free(c); } };

using unique_bio = std::unique_ptr<BIO, BIO_deleter>;
using unique_x509 = std::unique_ptr<X509, X509_deleter>;
using unique_ssl_ctx = std::unique_ptr<SSL_CTX, SSL_CTX_deleter>;

// RAII class to initialize and clean up OpenSSL library
class OpenSSLInitializer {
public:
    OpenSSLInitializer() {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
    }
    ~OpenSSLInitializer() {
        ERR_free_strings();
        EVP_cleanup();
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
};

bool checkCertificateHash(const std::string& host, int port, const std::string& knownHash) {
    unique_ssl_ctx ctx(SSL_CTX_new(TLS_client_method()));
    if (!ctx) {
        std::cerr << "  Error: Unable to create SSL context." << std::endl;
        return false;
    }

    unique_bio bio(BIO_new_ssl_connect(ctx.get()));
    if (!bio) {
        std::cerr << "  Error: Unable to create BIO." << std::endl;
        return false;
    }

    SSL* ssl = nullptr;
    BIO_get_ssl(bio.get(), &ssl);
    if (!ssl) {
        std::cerr << "  Error: Unable to get SSL pointer." << std::endl;
        return false;
    }

    // Enable SNI (Server Name Indication), crucial for shared hosting
    SSL_set_tlsext_host_name(ssl, host.c_str());

    std::string host_port = host + ":" + std::to_string(port);
    BIO_set_conn_hostname(bio.get(), host_port.c_str());

    if (BIO_do_connect(bio.get()) <= 0) {
        //fprintf(stderr, "  Error connecting to %s\n", host.c_str());
        //ERR_print_errors_fp(stderr);
        return false;
    }

    unique_x509 cert(SSL_get_peer_certificate(ssl));
    if (!cert) {
        std::cerr << "  Error: Could not get server certificate." << std::endl;
        return false;
    }

    // Get DER encoded certificate
    unsigned char* der_cert_buf = nullptr;
    int len = i2d_X509(cert.get(), &der_cert_buf);
    if (len < 0) {
        std::cerr << "  Error: Could not encode certificate to DER." << std::endl;
        return false;
    }
    std::unique_ptr<unsigned char, decltype(&OPENSSL_free)> der_cert_ptr(der_cert_buf, &OPENSSL_free);

    // Calculate SHA-256 hash
    unsigned char hash_buf[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    if (EVP_Digest(der_cert_ptr.get(), len, hash_buf, &hash_len, EVP_sha256(), NULL) != 1) {
        std::cerr << "  Error: Could not compute SHA-256 digest." << std::endl;
        return false;
    }

    std::string calculatedHash = bytesToHex(hash_buf, hash_len);
    return (strcasecmp(calculatedHash.c_str(), knownHash.c_str()) == 0);
}

int main() {
    OpenSSLInitializer ssl_initializer;
    
    // NOTE: These hashes are time-sensitive and may need to be updated as
    // websites rotate their SSL certificates. The hashes provided are for the
    // *entire* DER-encoded certificate, not just the public key.
    // To compile on Linux: g++ your_file.cpp -o your_program -lssl -lcrypto
    // On Windows, you may need to configure include/library paths and link against
    // libssl.lib, libcrypto.lib, ws2_32.lib etc.

    struct TestCase { std::string host; std::string hash; bool expected; };
    std::vector<TestCase> testCases = {
        {"google.com", "19f7a72675b331481b21cb4e488135891e4f34614207963d722108709328246a", true},
        {"github.com", "346b45b239094d2b2746487e452a2656973e6f21789c6d5b035f5022e032230a", true},
        {"example.com", "this_is_an_incorrect_hash_string_for_testing_failure1234567890", false},
        {"self-signed.badssl.com", "987724f28012626e205a91438b417c8227b403f752496a71e16c905f884a4411", true},
        {"thishostdoesnotexist12345.com", "any_hash_will_do_since_host_is_unreachable", false}
    };
    
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    int testNum = 1;
    for (const auto& test : testCases) {
        std::cout << "Test " << testNum++ << ": Checking host " << test.host << std::endl;
        bool result = checkCertificateHash(test.host, 443, test.hash);
        std::cout << "  Result: " << std::boolalpha << result << std::endl;
        std::cout << "  Expected: " << std::boolalpha << test.expected << std::endl;
        std::cout << "  Status: " << (result == test.expected ? "PASS" : "FAIL") << std::endl << std::endl;
    }
    
    return 0;
}