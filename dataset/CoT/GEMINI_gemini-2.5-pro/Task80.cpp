/*
 * C++ SSL Client for IPv6
 * Note: This code requires the OpenSSL library.
 * To compile: g++ your_file.cpp -o your_app -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

// POSIX/Linux headers for socket programming
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 * Initializes the OpenSSL library. Should be called once per application.
 */
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

/**
 * Cleans up the OpenSSL library resources.
 */
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates, connects, and performs the SSL handshake for an IPv6 client socket.
 * This function encapsulates the entire process from context creation to connection.
 *
 * @param host The hostname to connect to.
 * @param port The port number.
 * @return A pointer to the established SSL connection object.
 *         Returns nullptr on failure. The caller is responsible for a full cleanup on success.
 *         Full cleanup procedure:
 *         1. int sock_fd = SSL_get_fd(ssl);
 *         2. SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
 *         3. SSL_shutdown(ssl); // Graceful shutdown
 *         4. SSL_free(ssl);
 *         5. close(sock_fd);
 *         6. SSL_CTX_free(ctx);
 */
SSL* create_ssl_ipv6_client_socket(const std::string& host, int port) {
    SSL_CTX* ctx = nullptr;
    SSL* ssl = nullptr;
    int sock_fd = -1;

    // Create a new SSL context
    const SSL_METHOD* method = TLS_client_method();
    if (!method) {
        std::cerr << "Error: Unable to create SSL/TLS client method." << std::endl;
        return nullptr;
    }
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        std::cerr << "Error: Unable to create SSL context." << std::endl;
        return nullptr;
    }

    // Load default CA certificates from the system's trust store.
    // This is crucial for verifying the server's certificate.
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        std::cerr << "Error: Failed to load default CA certificates." << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return nullptr;
    }

    // Resolve hostname to IPv6 address
    struct addrinfo hints{}, *result;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
        std::cerr << "Error: getaddrinfo failed for host " << host << std::endl;
        SSL_CTX_free(ctx);
        return nullptr;
    }

    // Create and connect socket
    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock_fd == -1) continue;
        if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1) break; // Success
        close(sock_fd);
        sock_fd = -1;
    }
    freeaddrinfo(result);
    if (sock_fd == -1) {
        std::cerr << "Error: Could not create or connect IPv6 socket for " << host << std::endl;
        SSL_CTX_free(ctx);
        return nullptr;
    }

    // Create SSL object and associate it with the socket
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);

    // Set SNI (Server Name Indication) and enable hostname verification.
    if (SSL_set1_host(ssl, host.c_str()) != 1) {
        std::cerr << "Error: SSL_set1_host (SNI) failed." << std::endl;
        SSL_free(ssl); close(sock_fd); SSL_CTX_free(ctx);
        return nullptr;
    }

    // Perform the SSL/TLS handshake
    if (SSL_connect(ssl) != 1) {
        std::cerr << "Error: SSL/TLS handshake failed for host " << host << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl); close(sock_fd); SSL_CTX_free(ctx);
        return nullptr;
    }

    // Verify the server certificate result
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        std::cerr << "Error: Certificate verification failed for " << host << std::endl;
        SSL_free(ssl); close(sock_fd); SSL_CTX_free(ctx);
        return nullptr;
    }

    return ssl; // Success, caller must clean up all handles.
}

int main() {
    init_openssl();
    
    std::vector<std::string> testHosts = {
        "google.com",
        "www.facebook.com",
        "ipv6.google.com",
        "www.ietf.org",
        "badssl.com"
    };
    int port = 443;

    for (const auto& host : testHosts) {
        std::cout << "--- Testing connection to " << host << ":" << port << " ---" << std::endl;
        SSL* ssl = create_ssl_ipv6_client_socket(host, port);
        
        if (ssl) {
            std::cout << "Successfully connected to " << host << std::endl;
            std::cout << "SSL/TLS Protocol: " << SSL_get_version(ssl) << std::endl;
            std::cout << "Cipher Suite: " << SSL_get_cipher(ssl) << std::endl;
            
            // Full cleanup as documented in the function's comment
            int sock_fd = SSL_get_fd(ssl);
            SSL_CTX* ctx = SSL_get_SSL_CTX(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sock_fd);
            SSL_CTX_free(ctx);
        } else {
            std::cerr << "Connection test failed for " << host << std::endl;
        }
        std::cout << std::endl;
    }

    cleanup_openssl();
    return 0;
}