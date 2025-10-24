// Chain-of-Through process (high-level):
// 1) Build IPv6 TCP SSL client using OpenSSL.
// 2) Enforce certificate verification, hostname check, SNI, min TLS.
// 3) Implement safe connect with timeouts, cleanup on failure.
// 4) Review error paths and resource handling.
// 5) Provide final secure function and tests.

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>

static void openssl_init_once() {
    static bool inited = false;
    if (!inited) {
        OPENSSL_init_ssl(0, nullptr);
        inited = true;
    }
}

static int connect_ipv6_tcp(const std::string& host, int port, std::string& out_err) {
    struct addrinfo hints{};
    struct addrinfo* res = nullptr;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    std::string portStr = std::to_string(port);
    int rc = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (rc != 0) {
        out_err = gai_strerror(rc);
        return -1;
    }

    int fd = -1;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;

        // IPv6-only if possible
        int v6only = 1;
        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

        // Keepalive and TCP_NODELAY optional
        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));

        // Timeouts
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
            freeaddrinfo(res);
            return fd;
        }
        ::close(fd);
        fd = -1;
    }

    freeaddrinfo(res);
    out_err = "connect failed";
    return -1;
}

SSL* create_ipv6_ssl_client(const std::string& host, int port) {
    if (host.empty() || port <= 0 || port > 65535) {
        return nullptr;
    }
    openssl_init_once();

    std::string err;
    int fd = connect_ipv6_tcp(host, port, err);
    if (fd < 0) {
        return nullptr;
    }

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        ::close(fd);
        return nullptr;
    }

    // Minimum TLS 1.2
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    // Verify peer certificate
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    SSL_CTX_set_default_verify_paths(ctx);

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_CTX_free(ctx);
        ::close(fd);
        return nullptr;
    }

    // Hostname verification and SNI
    SSL_set_tlsext_host_name(ssl, host.c_str());
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    SSL_set1_host(ssl, host.c_str());
#endif

    if (SSL_set_fd(ssl, fd) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        ::close(fd);
        return nullptr;
    }

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        ::close(fd);
        return nullptr;
    }

    // SSL takes its own ref to ctx; safe to free our ref
    SSL_CTX_free(ctx);
    return ssl;
}

static void close_ssl(SSL* ssl) {
    if (!ssl) return;
    int fd = SSL_get_fd(ssl);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    if (fd >= 0) ::close(fd);
}

int main() {
    std::vector<std::string> hosts = {
        "google.com",
        "cloudflare.com",
        "example.com",
        "ietf.org",
        "wikipedia.org"
    };
    int port = 443;

    for (const auto& host : hosts) {
        std::cout << "Connecting to " << host << " over IPv6 TLS..." << std::endl;
        SSL* ssl = create_ipv6_ssl_client(host, port);
        if (!ssl) {
            std::cout << "Failed to connect to " << host << std::endl;
        } else {
            const char* ver = SSL_get_version(ssl);
            const char* cipher = SSL_get_cipher(ssl);
            std::cout << "Connected. TLS: " << (ver ? ver : "unknown")
                      << ", Cipher: " << (cipher ? cipher : "unknown") << std::endl;
        }
        close_ssl(ssl);
        std::cout << "----" << std::endl;
    }
    return 0;
}