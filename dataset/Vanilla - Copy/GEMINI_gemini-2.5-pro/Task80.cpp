/*
 * Compile with: g++ -o task80_cpp task80.cpp -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// A structure to hold the socket and SSL object
struct SslConnection {
    int socket_fd;
    SSL* ssl;
};

// Function to initialize the OpenSSL library
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Function to clean up OpenSSL
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates an SSL client socket using IPv6 and TCP.
 *
 * @param hostname The hostname to connect to.
 * @param port The port to connect to.
 * @return An SslConnection struct with the socket and SSL object, or {-1, nullptr} on failure.
 */
SslConnection create_ipv6_ssl_client_socket(const char* hostname, int port) {
    SSL_CTX *ctx = nullptr;
    int sock_fd = -1;
    SSL* ssl = nullptr;

    // Create SSL context
    const SSL_METHOD *method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        std::cerr << "Error: Unable to create SSL context." << std::endl;
        ERR_print_errors_fp(stderr);
        return {-1, nullptr};
    }

    // Get address info for IPv6
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, std::to_string(port).c_str(), &hints, &res) != 0) {
        std::cerr << "Error: getaddrinfo failed for host " << hostname << std::endl;
        SSL_CTX_free(ctx);
        return {-1, nullptr};
    }

    // Iterate through results and try to connect
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) {
            continue;
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == 0) {
            break; // Successfully connected
        }

        close(sock_fd);
        sock_fd = -1;
    }

    freeaddrinfo(res);

    if (sock_fd < 0) {
        std::cerr << "Error: Failed to connect to any IPv6 address for " << hostname << std::endl;
        SSL_CTX_free(ctx);
        return {-1, nullptr};
    }
    
    // Create SSL object and attach it to the socket
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);

    // Perform SSL/TLS handshake
    if (SSL_connect(ssl) <= 0) {
        std::cerr << "Error: SSL connection failed." << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock_fd);
        SSL_CTX_free(ctx);
        return {-1, nullptr};
    }
    
    SSL_CTX_free(ctx); // Context is no longer needed after SSL object creation
    return {sock_fd, ssl};
}

void close_connection(SslConnection conn) {
    if (conn.ssl) {
        SSL_shutdown(conn.ssl);
        SSL_free(conn.ssl);
    }
    if (conn.socket_fd >= 0) {
        close(conn.socket_fd);
    }
}

int main() {
    init_openssl();

    std::vector<std::string> hosts = {"google.com", "facebook.com", "ipv6.google.com", "wikipedia.org", "example.com"};
    int port = 443;

    for (const auto& host : hosts) {
        std::cout << "--- Testing connection to " << host << ":" << port << " ---" << std::endl;
        SslConnection conn = create_ipv6_ssl_client_socket(host.c_str(), port);

        if (conn.ssl) {
            std::cout << "Successfully connected to " << host << " over IPv6." << std::endl;
            std::cout << "Cipher: " << SSL_get_cipher(conn.ssl) << std::endl;
            close_connection(conn);
            std::cout << "Connection closed." << std::endl;
        } else {
            std::cout << "Failed to connect to " << host << " over IPv6." << std::endl;
        }
        std::cout << std::endl;
    }

    cleanup_openssl();
    return 0;
}