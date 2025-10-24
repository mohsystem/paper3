#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

static int set_nonblocking(int fd, bool on) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (on) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, int timeoutSec) {
    if (set_nonblocking(fd, true) < 0) return -1;
    int r = ::connect(fd, addr, addrlen);
    if (r == 0) {
        set_nonblocking(fd, false);
        return 0;
    }
    if (r < 0 && errno != EINPROGRESS) {
        set_nonblocking(fd, false);
        return -1;
    }
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    struct timeval tv;
    tv.tv_sec = timeoutSec;
    tv.tv_usec = 0;
    r = select(fd + 1, nullptr, &wfds, nullptr, &tv);
    if (r <= 0) {
        set_nonblocking(fd, false);
        errno = (r == 0) ? ETIMEDOUT : errno;
        return -1;
    }
    int so_error = 0;
    socklen_t len = sizeof(so_error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
        set_nonblocking(fd, false);
        return -1;
    }
    set_nonblocking(fd, false);
    if (so_error != 0) {
        errno = so_error;
        return -1;
    }
    return 0;
}

SSL* create_ipv6_ssl_socket(const std::string& host, const std::string& port, int timeoutSec) {
    if (host.empty() || host.find('\0') != std::string::npos) {
        return nullptr;
    }
    if (port.empty()) return nullptr;

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    struct addrinfo hints {};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo* res = nullptr;
    int gai = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (gai != 0) {
        return nullptr;
    }

    int fd = -1;
    SSL_CTX* ctx = nullptr;
    SSL* ssl = nullptr;
    for (struct addrinfo* rp = res; rp != nullptr; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;

        int one = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));

        if (connect_with_timeout(fd, rp->ai_addr, rp->ai_addrlen, timeoutSec) == 0) {
            break;
        }
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) {
        return nullptr;
    }

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        close(fd);
        return nullptr;
    }
#if defined(TLS1_2_VERSION)
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
#endif
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        SSL_CTX_free(ctx);
        close(fd);
        return nullptr;
    }

    X509_VERIFY_PARAM* param = SSL_CTX_get0_param(ctx);
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0) != 1) {
        SSL_CTX_free(ctx);
        close(fd);
        return nullptr;
    }

    ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_CTX_free(ctx);
        close(fd);
        return nullptr;
    }
    // SSL owns a ref to ctx now; free our reference
    SSL_CTX_free(ctx);

    SSL_set_tlsext_host_name(ssl, host.c_str());
    if (SSL_set_fd(ssl, fd) != 1) {
        SSL_free(ssl);
        close(fd);
        return nullptr;
    }
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        close(fd);
        return nullptr;
    }
    return ssl;
}

int main() {
    std::vector<std::string> hosts = {
        "ipv6.google.com",
        "www.cloudflare.com",
        "www.google.com",
        "www.wikipedia.org",
        "www.youtube.com"
    };
    for (const auto& h : hosts) {
        SSL* ssl = create_ipv6_ssl_socket(h, "443", 8);
        if (!ssl) {
            std::cout << "Failed to connect to " << h << std::endl;
            continue;
        }
        const SSL_CIPHER* cipher = SSL_get_current_cipher(ssl);
        std::cout << "Connected to " << h << " via IPv6, cipher=" << (cipher ? SSL_CIPHER_get_name(cipher) : "unknown") << std::endl;

        // Cleanup
        int fd = SSL_get_fd(ssl);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        if (fd >= 0) close(fd);
    }
    return 0;
}