/*
 * COMPILE: g++ Task105.cpp -o task105_cpp -lssl -lcrypto
 * RUN: ./task105_cpp
 * NOTE: Requires OpenSSL development libraries (e.g., on Ubuntu/Debian: sudo apt-get install libssl-dev).
 * This code uses POSIX sockets and will not compile on Windows without changes (e.g., using Winsock).
 */
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>

// OpenSSL headers
#include <openssl/ssl.h>
#include <openssl/err.h>

// Networking headers
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// RAII wrapper for global OpenSSL initialization. Ensures cleanup.
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

int create_and_connect_socket(const char* host, int port) {
    int sock_fd = -1;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, port_str, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(rv) << std::endl;
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock_fd);
            sock_fd = -1;
            continue;
        }
        break; 
    }
    
    freeaddrinfo(servinfo);
    return sock_fd;
}

bool establishSSLConnection(const std::string& host, int port) {
    std::cout << "Attempting to connect to " << host << ":" << port << "..." << std::endl;
    
    SSL_CTX* ctx = nullptr;
    SSL* ssl = nullptr;
    int server_fd = -1;
    bool success = false;
    
    try {
        ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            throw std::runtime_error("Failed to create SSL_CTX");
        }

        server_fd = create_and_connect_socket(host.c_str(), port);
        if (server_fd < 0) {
            throw std::runtime_error("Could not create or connect socket");
        }

        ssl = SSL_new(ctx);
        if (!ssl) {
            throw std::runtime_error("Failed to create SSL structure");
        }

        SSL_set_fd(ssl, server_fd);
        SSL_set_tlsext_host_name(ssl, host.c_str()); // Enable SNI

        if (SSL_connect(ssl) <= 0) {
            throw std::runtime_error("SSL handshake failed");
        }

        std::cout << "Successfully connected to " << host << ":" << port << std::endl;
        std::cout << "SSL Connection Info:" << std::endl;
        std::cout << "\tProtocol: " << SSL_get_version(ssl) << std::endl;
        std::cout << "\tCipher: " << SSL_get_cipher_name(ssl) << std::endl;
        
        success = true;

    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        if (ERR_peek_error() != 0) {
            ERR_print_errors_fp(stderr);
        }
    }
    
    if (ssl != nullptr) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (server_fd != -1) {
        close(server_fd);
    }
    if (ctx != nullptr) {
        SSL_CTX_free(ctx);
    }
    
    std::cout << "----------------------------------------" << std::endl;
    return success;
}

int main() {
    OpenSSLInitializer initializer; // Manages global init/cleanup

    // 5 Test Cases
    establishSSLConnection("google.com", 443);
    establishSSLConnection("github.com", 443);
    establishSSLConnection("cloudflare.com", 443);
    establishSSLConnection("amazon.com", 443);
    establishSSLConnection("nonexistent-host-for-testing.com", 443); // Failure case
    return 0;
}