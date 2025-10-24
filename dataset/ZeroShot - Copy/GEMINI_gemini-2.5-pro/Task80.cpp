/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against libssl and libcrypto.
 * Example compilation command on Linux/macOS:
 * g++ Task80.cpp -o Task80 -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Global setup for OpenSSL, called once.
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// Global cleanup for OpenSSL, called once.
void cleanup_openssl() {
    EVP_cleanup();
}

/**
 * Creates a secure SSL client socket using IPv6 and connects to the specified host and port.
 *
 * @param hostname The hostname to connect to.
 * @param port The port number to connect to.
 * @return A pointer to the SSL structure on success, or nullptr on failure.
 *         The caller is responsible for freeing the SSL and SSL_CTX objects and closing the socket.
 */
SSL* create_ipv6_ssl_socket(const char* hostname, int port) {
    SSL_CTX *ctx = nullptr;
    SSL *ssl = nullptr;
    int sock_fd = -1;

    // 1. Create SSL Context
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        std::cerr << "Error: Unable to create SSL context." << std::endl;
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    // Load default trust store for certificate validation
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        std::cerr << "Error: Failed to load default verify paths." << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return nullptr;
    }

    // 2. Resolve hostname to IPv6 address and connect
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, std::to_string(port).c_str(), &hints, &res) != 0) {
        std::cerr << "Error: Cannot resolve IPv6 address for " << hostname << std::endl;
        SSL_CTX_free(ctx);
        return nullptr;
    }
    
    std::cout << "Resolved IPv6 address for " << hostname << std::endl;
    sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd < 0) {
        std::cerr << "Error: Unable to create socket." << std::endl;
        freeaddrinfo(res);
        SSL_CTX_free(ctx);
        return nullptr;
    }

    if (connect(sock_fd, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Error: Cannot connect to " << hostname << ":" << port << std::endl;
        perror("connect");
        close(sock_fd);
        freeaddrinfo(res);
        SSL_CTX_free(ctx);
        return nullptr;
    }
    freeaddrinfo(res);

    // 3. Create SSL structure and perform handshake
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);

    // Set SNI (Server Name Indication) - crucial for many modern servers
    SSL_set_tlsext_host_name(ssl, hostname);

    if (SSL_connect(ssl) <= 0) {
        std::cerr << "Error: SSL handshake failed with " << hostname << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl); // This also closes the underlying socket fd
        SSL_CTX_free(ctx);
        return nullptr;
    }

    std::cout << "SSL handshake successful with " << hostname << ":" << port << std::endl;
    SSL_CTX_free(ctx); // Context can be freed after SSL object creation
    return ssl;
}

int main() {
    init_openssl();

    // --- Test Cases ---
    std::vector<std::pair<std::string, int>> test_cases = {
        {"google.com", 443},
        {"facebook.com", 443},
        {"wikipedia.org", 443},
        {"nonexistent.veryunlikelydomain.com", 443},
        {"google.com", 444}
    };

    for (int i = 0; i < test_cases.size(); ++i) {
        const auto& test_case = test_cases[i];
        const char* host = test_case.first.c_str();
        int port = test_case.second;

        std::cout << "\n--- Test Case " << (i + 1) << ": Connecting to " << host << ":" << port << " ---" << std::endl;

        SSL* ssl = create_ipv6_ssl_socket(host, port);

        if (ssl) {
            std::cout << "Successfully connected to " << host << ":" << port << std::endl;
            
            // Perform a simple HTTP GET to verify connection
            std::string request = "GET / HTTP/1.1\r\nHost: " + std::string(host) + "\r\nConnection: close\r\n\r\n";
            SSL_write(ssl, request.c_str(), request.length());

            char buffer[1024] = {0};
            int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                std::string response(buffer);
                size_t first_line_end = response.find("\r\n");
                if (first_line_end != std::string::npos) {
                    std::cout << "Response from server: " << response.substr(0, first_line_end) << std::endl;
                }
            }
            
            // Cleanup
            int sock_fd = SSL_get_fd(ssl);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sock_fd);
        } else {
            std::cout << "Failed to connect to " << host << ":" << port << std::endl;
        }
    }

    cleanup_openssl();
    return 0;
}