#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <vector>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// RAII wrappers for OpenSSL resources
using SSL_CTX_ptr = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;
using SSL_ptr = std::unique_ptr<SSL, decltype(&SSL_free)>;
using BIO_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;
using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;


void print_ssl_error() {
    BIO_ptr bio(BIO_new(BIO_s_mem()), BIO_free);
    ERR_print_errors(bio.get());
    char* buf;
    size_t len = BIO_get_mem_data(bio.get(), &buf);
    std::cerr << "OpenSSL Error: " << std::string(buf, len) << std::endl;
}

// Initializes OpenSSL library
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

int create_tcp_socket(const char* host, int port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, std::to_string(port).c_str(), &hints, &res) != 0) {
        std::cerr << "Error: getaddrinfo failed for " << host << std::endl;
        return -1;
    }

    int sockfd = -1;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break; 
    }
    
    freeaddrinfo(res);
    if (sockfd == -1) {
        std::cerr << "Error: Failed to create and connect socket." << std::endl;
    }
    return sockfd;
}

bool establishSecureConnection(const char* host, int port) {
    if (!host || std::strlen(host) == 0 || port <= 0 || port > 65535) {
        std::cerr << "Invalid host or port." << std::endl;
        return false;
    }

    SSL_CTX_ptr ctx(SSL_CTX_new(TLS_client_method()), SSL_CTX_free);
    if (!ctx) {
        print_ssl_error();
        return false;
    }
    
    // Load default trusted CAs from system
    if (SSL_CTX_set_default_verify_paths(ctx.get()) != 1) {
        std::cerr << "Error: Failed to load default trusted CAs." << std::endl;
        print_ssl_error();
        return false;
    }

    int sockfd = create_tcp_socket(host, port);
    if (sockfd == -1) {
        return false;
    }

    SSL_ptr ssl(SSL_new(ctx.get()), SSL_free);
    if (!ssl) {
        print_ssl_error();
        close(sockfd);
        return false;
    }

    SSL_set_fd(ssl.get(), sockfd);

    // Enable hostname verification. This is critical for security.
    if (SSL_set1_host(ssl.get(), host) != 1) {
        std::cerr << "Error: Failed to set hostname for verification." << std::endl;
        print_ssl_error();
        close(sockfd);
        return false;
    }

    if (SSL_connect(ssl.get()) != 1) {
        std::cerr << "SSL/TLS connection failed for " << host << ":" << port << std::endl;
        print_ssl_error();
        close(sockfd);
        return false;
    }

    // Verify the server certificate
    long verify_result = SSL_get_verify_result(ssl.get());
    if (verify_result != X509_V_OK) {
        std::cerr << "Certificate verification failed for " << host << ": "
                  << X509_verify_cert_error_string(verify_result) << std::endl;
        close(sockfd);
        return false;
    }

    std::cout << "Successfully connected to " << host << ":" << port << std::endl;
    std::cout << "SSL/TLS Protocol: " << SSL_get_version(ssl.get()) << std::endl;
    std::cout << "Cipher Suite: " << SSL_get_cipher_name(ssl.get()) << std::endl;

    X509_ptr cert(SSL_get_peer_certificate(ssl.get()), X509_free);
    if (cert) {
        char* subject = X509_NAME_oneline(X509_get_subject_name(cert.get()), nullptr, 0);
        std::cout << "Peer Certificate: " << subject << std::endl;
        OPENSSL_free(subject);
    }
    
    // Example: Send HTTP GET request
    std::string request = "GET / HTTP/1.1\r\nHost: " + std::string(host) + "\r\nConnection: close\r\n\r\n";
    SSL_write(ssl.get(), request.c_str(), request.length());

    char buffer[4096];
    int bytes = SSL_read(ssl.get(), buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = 0;
        std::string response(buffer);
        size_t end_pos = response.find("\r\n\r\n");
        if (end_pos != std::string::npos) {
           response = response.substr(0, end_pos);
        }
        std::cout << "\n--- Server Response (first few lines) ---\n";
        std::cout << response.substr(0, 200) << "...\n";
        std::cout << "---------------------------------------\n" << std::endl;
    }


    SSL_shutdown(ssl.get());
    close(sockfd);
    return true;
}

int main() {
    OpenSSLInitializer initializer;

    const char* hosts[] = {
        "www.google.com",
        "www.cloudflare.com",
        "expired.badssl.com",
        "wrong.host.badssl.com",
        "self-signed.badssl.com"
    };
    int port = 443;

    for (const char* host : hosts) {
        std::cout << "--- Testing connection to " << host << " ---" << std::endl;
        bool success = establishSecureConnection(host, port);
        std::cout << "Connection status: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
        std::cout << "--------------------------------------\n" << std::endl;
    }

    return 0;
}