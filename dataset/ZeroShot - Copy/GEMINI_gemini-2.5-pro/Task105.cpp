#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// Socket headers (for POSIX systems)
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// Note: To compile this code, you need to link against OpenSSL libraries.
// Example command: g++ Task105.cpp -o Task105 -lssl -lcrypto

// --- RAII Wrappers for automatic resource management ---

// Custom deleter for SSL_CTX
struct SSL_CTX_deleter {
    void operator()(SSL_CTX* ctx) { if (ctx) SSL_CTX_free(ctx); }
};
using unique_SSL_CTX = std::unique_ptr<SSL_CTX, SSL_CTX_deleter>;

// Custom deleter for SSL
struct SSL_deleter {
    void operator()(SSL* ssl) { if (ssl) SSL_free(ssl); }
};
using unique_SSL = std::unique_ptr<SSL, SSL_deleter>;

// Custom deleter for a socket file descriptor
struct Socket_deleter {
    void operator()(int* fd) { if (fd && *fd != -1) { close(*fd); delete fd; } }
};
using unique_Socket = std::unique_ptr<int, Socket_deleter>;


// A simple class to initialize and clean up OpenSSL library globally
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

bool establishSecureConnection(const std::string& host, int port) {
    unique_SSL_CTX ctx(SSL_CTX_new(TLS_client_method()));
    if (!ctx) {
        std::cerr << "Error creating SSL_CTX." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Set secure options: disable insecure protocols like SSLv2, SSLv3, TLSv1, TLSv1.1
    SSL_CTX_set_options(ctx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    // Load default trusted CA certificates from system-specific paths
    if (SSL_CTX_set_default_verify_paths(ctx.get()) != 1) {
        std::cerr << "Error loading default CA certificates." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    // TCP connection using getaddrinfo for IPv4/IPv6 compatibility
    struct addrinfo hints, *res, *p;
    int raw_sock_fd = -1;
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);
    
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), port_str, &hints, &res) != 0) {
        std::cerr << "getaddrinfo error for host: " << host << std::endl;
        return false;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        raw_sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (raw_sock_fd < 0) continue;
        if (connect(raw_sock_fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(raw_sock_fd);
        raw_sock_fd = -1;
    }
    freeaddrinfo(res);

    if (raw_sock_fd < 0) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        return false;
    }
    unique_Socket sock_fd(new int(raw_sock_fd));

    // Create SSL object and attach it to the socket
    unique_SSL ssl(SSL_new(ctx.get()));
    if (!ssl) {
        std::cerr << "Error creating SSL object." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    SSL_set_fd(ssl.get(), *sock_fd);

    // Enable SNI (Server Name Indication) - crucial for modern HTTPS
    if (SSL_set_tlsext_host_name(ssl.get(), host.c_str()) != 1) {
       std::cerr << "Error setting SNI hostname." << std::endl;
       ERR_print_errors_fp(stderr);
       return false;
    }
    
    // Enable hostname verification
    X509_VERIFY_PARAM *param = SSL_get0_param(ssl.get());
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (!X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0)) {
        std::cerr << "Error setting hostname for verification." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }
    SSL_set_verify(ssl.get(), SSL_VERIFY_PEER, NULL);

    // Perform the SSL/TLS handshake
    if (SSL_connect(ssl.get()) <= 0) {
        std::cerr << "SSL Handshake Failed for " << host << ":" << port << "." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Verify the server's certificate
    long verify_result = SSL_get_verify_result(ssl.get());
    if (verify_result != X509_V_OK) {
        std::cerr << "Certificate verification failed for " << host << ":" << port << ". Error: "
                  << X509_verify_cert_error_string(verify_result) << std::endl;
        return false;
    }

    std::cout << "Successfully connected to " << host << ":" << port << std::endl;
    std::cout << "SSL Session Info:" << std::endl;
    std::cout << "\tProtocol: " << SSL_get_version(ssl.get()) << std::endl;
    std::cout << "\tCipher Suite: " << SSL_get_cipher(ssl.get()) << std::endl;
    
    SSL_shutdown(ssl.get());
    
    return true;
}

int main() {
    OpenSSLInitializer ssl_initializer;

    std::vector<std::pair<std::string, int>> test_cases = {
        {"google.com", 443},
        {"github.com", 443},
        {"expired.badssl.com", 443},
        {"wrong.host.badssl.com", 443},
        {"nonexistent-domain-xyz123.com", 443}
    };

    int i = 1;
    for (const auto& test_case : test_cases) {
        std::cout << "\n--- Test Case " << i++ << ": Connecting to " << test_case.first << ":" << test_case.second << " ---" << std::endl;
        bool success = establishSecureConnection(test_case.first, test_case.second);
        std::cout << "Connection status: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
        std::cout << "----------------------------------------------------" << std::endl;
    }

    return 0;
}