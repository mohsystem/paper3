#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

// RAII wrappers for OpenSSL types
using BioPtr = std::unique_ptr<BIO, decltype(&BIO_free_all)>;
using X509Ptr = std::unique_ptr<X509, decltype(&X509_free)>;
using EvpMdCtxPtr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

// Helper to convert byte array to hex string
std::string bytesToHex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return ss.str();
}

// Case-insensitive string comparison
bool iequals(const std::string& a, const std::string& b) {
    return a.length() == b.length() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
               return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2));
           });
}

bool checkCertificateHash(const std::string& host, int port, const std::string& knownHash, const std::string& algorithm) {
    if (host.empty() || knownHash.empty() || algorithm.empty()) {
        return false;
    }

    std::string host_port = host + ":" + std::to_string(port);
    BioPtr bio(BIO_new_ssl_connect(nullptr), &BIO_free_all);
    if (!bio) {
        std::cerr << "Error: BIO_new_ssl_connect failed." << std::endl;
        return false;
    }

    BIO_set_conn_hostname(bio.get(), host_port.c_str());

    SSL* ssl = nullptr;
    BIO_get_ssl(bio.get(), &ssl);
    if (!ssl) {
        std::cerr << "Error: BIO_get_ssl failed." << std::endl;
        return false;
    }

    // Set SNI, which is crucial for virtual hosting
    SSL_set_tlsext_hostname(ssl, host.c_str());

    if (BIO_do_connect(bio.get()) <= 0) {
        std::cerr << "Error: Could not connect to " << host << std::endl;
        return false;
    }

    if (BIO_do_handshake(bio.get()) <= 0) {
        std::cerr << "Error: SSL handshake failed for " << host << std::endl;
        return false;
    }

    X509Ptr cert(SSL_get_peer_certificate(ssl), &X509_free);
    if (!cert) {
        std::cerr << "Error: Could not get peer certificate from " << host << std::endl;
        return false;
    }

    unsigned char* der_buf = nullptr;
    int der_len = i2d_X509(cert.get(), &der_buf);
    if (der_len < 0) {
        std::cerr << "Error: i2d_X509 failed." << std::endl;
        return false;
    }
    std::vector<unsigned char> cert_der(der_buf, der_buf + der_len);
    OPENSSL_free(der_buf);

    const EVP_MD* md = EVP_get_digestbyname(algorithm.c_str());
    if (!md) {
        std::cerr << "Error: Unknown message digest " << algorithm << std::endl;
        return false;
    }

    EvpMdCtxPtr mdctx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
    if (!mdctx) {
        std::cerr << "Error: EVP_MD_CTX_new failed." << std::endl;
        return false;
    }

    if (1 != EVP_DigestInit_ex(mdctx.get(), md, nullptr)) {
        std::cerr << "Error: EVP_DigestInit_ex failed." << std::endl;
        return false;
    }

    if (1 != EVP_DigestUpdate(mdctx.get(), cert_der.data(), cert_der.size())) {
        std::cerr << "Error: EVP_DigestUpdate failed." << std::endl;
        return false;
    }

    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    if (1 != EVP_DigestFinal_ex(mdctx.get(), md_value, &md_len)) {
        std::cerr << "Error: EVP_DigestFinal_ex failed." << std::endl;
        return false;
    }

    std::string calculatedHash = bytesToHex(md_value, md_len);
    
    return iequals(calculatedHash, knownHash);
}

// Helper for running and reporting tests
void run_test(const std::string& name, bool result, bool expected) {
    std::cout << name << ": " << (result == expected ? "PASS" : "FAIL") << std::endl;
}

int main() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    // NOTE: These hashes are time-sensitive and will change when certificates are renewed.
    // You may need to update them for the tests to pass.
    const std::string GOOGLE_SHA256 = "1a3b865582e022f42a6c8e317c223c3b0693a61f2382855140b28e57973c150c";
    const std::string GITHUB_SHA256 = "1980318288b3986a420f4c0842271676f45237c95a25e6215b22b069d2c55b9e";
    const std::string FAKE_HASH = "0000000000000000000000000000000000000000000000000000000000000000";

    std::cout << "Running test cases..." << std::endl;

    bool test1 = checkCertificateHash("www.google.com", 443, GOOGLE_SHA256, "sha256");
    run_test("1. www.google.com with correct hash", test1, true);
    
    bool test2 = checkCertificateHash("www.google.com", 443, FAKE_HASH, "sha256");
    run_test("2. www.google.com with incorrect hash", test2, false);

    bool test3 = checkCertificateHash("www.github.com", 443, GOOGLE_SHA256, "sha256");
    run_test("3. www.github.com with google.com hash", test3, false);

    bool test4 = checkCertificateHash("invalid.hostname.that.does.not.exist", 443, FAKE_HASH, "sha256");
    run_test("4. Invalid hostname", test4, false);

    bool test5 = checkCertificateHash("www.github.com", 443, GITHUB_SHA256, "sha256");
    run_test("5. www.github.com with correct hash", test5, true);

    EVP_cleanup();
    ERR_free_strings();

    return 0;
}