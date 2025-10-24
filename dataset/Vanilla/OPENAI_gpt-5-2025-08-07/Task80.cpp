#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

static SSL_CTX* get_global_ctx() {
    static SSL_CTX* ctx = nullptr;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        const SSL_METHOD* method = TLS_client_method();
        ctx = SSL_CTX_new(method);
        if (ctx) {
            SSL_CTX_set_default_verify_paths(ctx);
        }
    }
    return ctx;
}

SSL* create_ipv6_ssl_client(const std::string& host, int port) {
    SSL_CTX* ctx = get_global_ctx();
    if (!ctx) return nullptr;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* res = nullptr;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    int gai = getaddrinfo(host.c_str(), portstr, &hints, &res);
    if (gai != 0) {
        return nullptr;
    }

    int fd = -1;
    for (struct addrinfo* ai = res; ai != nullptr; ai = ai->ai_next) {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
            break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    if (fd < 0) {
        return nullptr;
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        close(fd);
        return nullptr;
    }

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
    SSL_set_tlsext_host_name(ssl, host.c_str());
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    SSL_set1_host(ssl, host.c_str());
#endif
    SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);
    SSL_set_fd(ssl, fd);

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        close(fd);
        return nullptr;
    }

    return ssl;
}

int main() {
    std::vector<std::string> hosts = {
        "google.com",
        "cloudflare.com",
        "example.com",
        "wikipedia.org",
        "ietf.org"
    };

    for (const auto& h : hosts) {
        SSL* ssl = create_ipv6_ssl_client(h, 443);
        if (ssl) {
            const SSL_CIPHER* cipher = SSL_get_current_cipher(ssl);
            std::cout << "Connected to " << h << " via IPv6 using " 
                      << SSL_get_version(ssl) << " "
                      << (cipher ? SSL_CIPHER_get_name(cipher) : "unknown") << std::endl;
            int fd = SSL_get_fd(ssl);
            SSL_free(ssl);
            if (fd >= 0) close(fd);
        } else {
            std::cout << "Failed to connect to " << h << std::endl;
        }
    }

    return 0;
}