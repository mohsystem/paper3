/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile on Linux/macOS:
 * g++ your_file_name.cpp -o your_executable -lssl -lcrypto
 *
 * On Windows, you will need to install OpenSSL and configure your compiler
 * to link against the necessary libraries (e.g., libssl.lib, libcrypto.lib).
 */
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// Socket-related headers (POSIX)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// Custom deleters for smart pointers to handle OpenSSL resource cleanup (RAII)
struct SSL_CTX_deleter {
    void operator()(SSL_CTX* ctx) const { if (ctx) SSL_CTX_free(ctx); }
};
struct SSL_deleter {
    void operator()(SSL* ssl) const { if (ssl) SSL_free(ssl); }
};

using SSL_CTX_ptr = std::unique_ptr<SSL_CTX, SSL_CTX_deleter>;
using SSL_ptr = std::unique_ptr<SSL, SSL_deleter>;

/**
 * Initializes the OpenSSL library. Should be called once per application.
 */
void init_openssl() {
    // These are thread-safe and can be called multiple times.
    // In OpenSSL 1.1.0+, this is done automatically.
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

/**
 * Creates a TCP connection to the specified host and port.
 * @return The socket file descriptor, or -1 on failure.
 */
int create_tcp_socket(const char* host, int port) {
    struct hostent* he;
    if ((he = gethostbyname(host)) == nullptr) {
        std::cerr << "ERROR: Could not resolve hostname: " << host << std::endl;
        return -1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("ERROR: Could not create socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr*)he->h_addr_list[0]);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Could not connect to host");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

/**
 * Establishes a secure SSL/TLS connection to a remote server.
 *
 * @param host The hostname of the server.
 * @param port The port number of the server.
 * @return true if the connection was successfully established, false otherwise.
 */
bool establishTlsConnection(const std::string& host, int port) {
    std::cout << "Attempting to connect to " << host << ":" << port << " over TLS..." << std::endl;

    // Create SSL context. TLS_client_method() is the modern, preferred method.
    SSL_CTX_ptr ctx(SSL_CTX_new(TLS_client_method()));
    if (!ctx) {
        std::cerr << "ERROR: Could not create SSL context." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Load default trusted CA certificates from the system's standard locations.
    // This is crucial for server certificate verification.
    if (SSL_CTX_set_default_verify_paths(ctx.get()) != 1) {
        std::cerr << "ERROR: Could not load default CA certificates." << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    int sock_fd = create_tcp_socket(host.c_str(), port);
    if (sock_fd == -1) {
        return false;
    }

    SSL_ptr ssl(SSL_new(ctx.get()));
    if (!ssl) {
        std::cerr << "ERROR: Could not create SSL structure." << std::endl;
        ERR_print_errors_fp(stderr);
        close(sock_fd);
        return false;
    }

    // Attach the SSL structure to the socket file descriptor
    SSL_set_fd(ssl.get(), sock_fd);
    
    // Set the Server Name Indication (SNI) extension. This is critical for
    // many modern web servers that host multiple sites on one IP address.
    if (SSL_set_tlsext_host_name(ssl.get(), host.c_str()) != 1) {
        std::cerr << "ERROR: Could not set SNI hostname." << std::endl;
        ERR_print_errors_fp(stderr);
        close(sock_fd);
        return false;
    }
    
    // Enable hostname verification. This checks that the certificate's
    // Common Name or Subject Alternative Name matches the host we are connecting to.
    X509_VERIFY_PARAM* param = SSL_get0_param(ssl.get());
    X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0);
    SSL_set_verify(ssl.get(), SSL_VERIFY_PEER, nullptr);

    // Perform the TLS handshake
    if (SSL_connect(ssl.get()) != 1) {
        std::cerr << "ERROR: TLS handshake failed." << std::endl;
        ERR_print_errors_fp(stderr);
        close(sock_fd);
        return false;
    }

    // After a successful handshake, it's good practice to double-check the
    // certificate verification result.
    long verify_result = SSL_get_verify_result(ssl.get());
    if (verify_result != X509_V_OK) {
        std::cerr << "ERROR: Certificate verification failed with code: " << verify_result << std::endl;
        std::cerr << "Reason: " << X509_verify_cert_error_string(verify_result) << std::endl;
        close(sock_fd);
        return false;
    }

    std::cout << "TLS connection established successfully!" << std::endl;
    std::cout << "Using protocol: " << SSL_get_version(ssl.get()) << std::endl;
    std::cout << "Using cipher: " << SSL_get_cipher(ssl.get()) << std::endl;

    // Cleanly shut down the connection
    SSL_shutdown(ssl.get());
    close(sock_fd);

    return true;
}

int main() {
    init_openssl();

    std::cout << "--- Running SSL/TLS Connection Tests ---" << std::endl;

    // Test Case 1
    std::cout << "\n[Test Case 1: www.google.com]" << std::endl;
    bool result1 = establishTlsConnection("www.google.com", 443);
    std::cout << "Test 1 Result: " << (result1 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 2
    std::cout << "\n[Test Case 2: www.github.com]" << std::endl;
    bool result2 = establishTlsConnection("www.github.com", 443);
    std::cout << "Test 2 Result: " << (result2 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 3
    std::cout << "\n[Test Case 3: expired.badssl.com]" << std::endl;
    bool result3 = establishTlsConnection("expired.badssl.com", 443);
    std::cout << "Test 3 Result: " << (!result3 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)") << std::endl;

    // Test Case 4
    std::cout << "\n[Test Case 4: wrong.host.badssl.com]" << std::endl;
    bool result4 = establishTlsConnection("wrong.host.badssl.com", 443);
    std::cout << "Test 4 Result: " << (!result4 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)") << std::endl;

    // Test Case 5
    std::cout << "\n[Test Case 5: non-existent-domain-for-testing-123.com]" << std::endl;
    bool result5 = establishTlsConnection("non-existent-domain-for-testing-123.com", 443);
    std::cout << "Test 5 Result: " << (!result5 ? "SUCCESS (Correctly failed)" : "FAILURE (Incorrectly succeeded)") << std::endl;

    std::cout << "\n--- All tests completed. ---" << std::endl;

    return 0;
}