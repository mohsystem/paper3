#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <memory>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// Custom deleters for OpenSSL and addrinfo to be used with std::unique_ptr for RAII
struct SSL_CTX_deleter { void operator()(SSL_CTX* ctx) const { if (ctx) SSL_CTX_free(ctx); } };
struct SSL_deleter { void operator()(SSL* ssl) const { if (ssl) SSL_free(ssl); } };
struct addrinfo_deleter { void operator()(struct addrinfo* ai) const { if (ai) freeaddrinfo(ai); } };

using unique_SSL_CTX = std::unique_ptr<SSL_CTX, SSL_CTX_deleter>;
using unique_SSL = std::unique_ptr<SSL, SSL_deleter>;
using unique_addrinfo = std::unique_ptr<struct addrinfo, addrinfo_deleter>;

// Helper function to print the OpenSSL error stack
void print_openssl_errors() {
    unsigned long err_code;
    while ((err_code = ERR_get_error())) {
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        std::cerr << "OpenSSL error: " << err_buf << std::endl;
    }
}

// RAII wrapper for an SSL connection and its underlying socket
struct SslConnection {
    unique_SSL ssl;
    int socket_fd;

    SslConnection() : socket_fd(-1) {}
    ~SslConnection() {
        if (ssl) SSL_shutdown(ssl.get());
        if (socket_fd != -1) close(socket_fd);
    }
    // Disable copy, enable move
    SslConnection(const SslConnection&) = delete;
    SslConnection& operator=(const SslConnection&) = delete;
    SslConnection(SslConnection&& other) noexcept : ssl(std::move(other.ssl)), socket_fd(other.socket_fd) {
        other.socket_fd = -1;
    }
    SslConnection& operator=(SslConnection&& other) noexcept {
        if (this != &other) {
            ssl = std::move(other.ssl);
            socket_fd = other.socket_fd;
            other.socket_fd = -1;
        }
        return *this;
    }
};

SslConnection create_ssl_ipv6_socket(const std::string& host, int port) {
    if (port <= 0 || port > 65535) {
        throw std::invalid_argument("Port out of valid range (1-65535)");
    }
    
    unique_SSL_CTX ctx(SSL_CTX_new(TLS_client_method()));
    if (!ctx) {
        print_openssl_errors();
        throw std::runtime_error("Failed to create SSL_CTX");
    }

    // Load default trusted CA certificates for server verification
    if (SSL_CTX_set_default_verify_paths(ctx.get()) != 1) {
        print_openssl_errors();
        throw std::runtime_error("Failed to load default CA certificates");
    }

    struct addrinfo hints{}, *result;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
        throw std::runtime_error("getaddrinfo failed for host: " + host);
    }
    unique_addrinfo ai_res(result);

    int sock_fd = -1;
    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock_fd == -1) continue;
        if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sock_fd);
        sock_fd = -1;
    }

    if (sock_fd == -1) {
        throw std::runtime_error("Failed to connect to any resolved IPv6 address for " + host);
    }

    unique_SSL ssl(SSL_new(ctx.get()));
    if (!ssl) {
        close(sock_fd);
        print_openssl_errors();
        throw std::runtime_error("Failed to create SSL structure");
    }
    
    SSL_set_fd(ssl.get(), sock_fd);
    SSL_set_tlsext_host_name(ssl.get(), host.c_str()); // Set SNI
    SSL_set1_host(ssl.get(), host.c_str()); // Set hostname for verification
    SSL_set_verify(ssl.get(), SSL_VERIFY_PEER, nullptr);

    if (SSL_connect(ssl.get()) != 1) {
        close(sock_fd);
        print_openssl_errors();
        throw std::runtime_error("SSL handshake failed with " + host);
    }

    SslConnection conn;
    conn.ssl = std::move(ssl);
    conn.socket_fd = sock_fd;
    return conn;
}

int main() {
    OPENSSL_init_ssl(0, NULL);

    std::vector<std::pair<std::string, int>> test_cases = {
        {"google.com", 443}, {"ietf.org", 443}, {"wikipedia.org", 443},
        {"thishostdoesnotexist12345.com", 443}, {"google.com", 99999}
    };

    for (size_t i = 0; i < test_cases.size(); ++i) {
        const auto& test = test_cases[i];
        std::cout << "--- Test Case " << i + 1 << ": Connecting to " << test.first << ":" << test.second << " over IPv6 ---" << std::endl;
        try {
            SslConnection conn = create_ssl_ipv6_socket(test.first, test.second);
            std::cout << "Successfully connected to " << test.first << ":" << test.second << std::endl;
            std::cout << "SSL Protocol: " << SSL_get_version(conn.ssl.get()) << std::endl;
            std::cout << "Cipher Suite: " << SSL_get_cipher_name(conn.ssl.get()) << std::endl;
            
            std::string request = "GET / HTTP/1.1\r\nHost: " + test.first + "\r\nConnection: close\r\n\r\n";
            SSL_write(conn.ssl.get(), request.c_str(), static_cast<int>(request.length()));

            char buffer[1024];
            int bytes = SSL_read(conn.ssl.get(), buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::string response(buffer);
                size_t first_line_end = response.find("\r\n");
                std::cout << "Server response (first line): " << response.substr(0, first_line_end) << std::endl;
            }
            // SslConnection destructor handles cleanup automatically
        } catch (const std::exception& e) {
            std::cerr << "Failed to connect or communicate. Reason: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}