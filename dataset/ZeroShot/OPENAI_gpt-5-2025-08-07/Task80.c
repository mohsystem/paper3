#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

static int set_nonblocking(int fd, int on) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (on) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, int timeoutSec) {
    if (set_nonblocking(fd, 1) < 0) return -1;
    int r = connect(fd, addr, addrlen);
    if (r == 0) {
        set_nonblocking(fd, 0);
        return 0;
    }
    if (r < 0 && errno != EINPROGRESS) {
        set_nonblocking(fd, 0);
        return -1;
    }
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    struct timeval tv;
    tv.tv_sec = timeoutSec;
    tv.tv_usec = 0;
    r = select(fd + 1, NULL, &wfds, NULL, &tv);
    if (r <= 0) {
        set_nonblocking(fd, 0);
        errno = (r == 0) ? ETIMEDOUT : errno;
        return -1;
    }
    int so_error = 0;
    socklen_t len = sizeof(so_error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
        set_nonblocking(fd, 0);
        return -1;
    }
    set_nonblocking(fd, 0);
    if (so_error != 0) {
        errno = so_error;
        return -1;
    }
    return 0;
}

SSL* create_ipv6_ssl_socket(const char* host, const char* port, int timeoutSec) {
    if (!host || !port || strchr(host, '\0') == NULL) {
        // host is null or contains embedded nulls are indistinguishable here; assume host non-empty
        ;
    }
    if (!host || !*host || !port || !*port) return NULL;

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* res = NULL;
    int gai = getaddrinfo(host, port, &hints, &res);
    if (gai != 0) {
        return NULL;
    }

    int fd = -1;
    for (struct addrinfo* rp = res; rp != NULL; rp = rp->ai_next) {
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
        return NULL;
    }

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        close(fd);
        return NULL;
    }
#if defined(TLS1_2_VERSION)
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
#endif
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        SSL_CTX_free(ctx);
        close(fd);
        return NULL;
    }

    X509_VERIFY_PARAM* param = SSL_CTX_get0_param(ctx);
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (X509_VERIFY_PARAM_set1_host(param, host, 0) != 1) {
        SSL_CTX_free(ctx);
        close(fd);
        return NULL;
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_CTX_free(ctx);
        close(fd);
        return NULL;
    }
    SSL_CTX_free(ctx); // SSL holds a ref

    SSL_set_tlsext_host_name(ssl, host);
    if (SSL_set_fd(ssl, fd) != 1) {
        SSL_free(ssl);
        close(fd);
        return NULL;
    }
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        close(fd);
        return NULL;
    }
    return ssl;
}

int main(void) {
    const char* hosts[5] = {
        "ipv6.google.com",
        "www.cloudflare.com",
        "www.google.com",
        "www.wikipedia.org",
        "www.youtube.com"
    };
    for (int i = 0; i < 5; ++i) {
        SSL* ssl = create_ipv6_ssl_socket(hosts[i], "443", 8);
        if (!ssl) {
            printf("Failed to connect to %s\n", hosts[i]);
            continue;
        }
        const SSL_CIPHER* cipher = SSL_get_current_cipher(ssl);
        printf("Connected to %s via IPv6, cipher=%s\n", hosts[i], cipher ? SSL_CIPHER_get_name(cipher) : "unknown");
        int fd = SSL_get_fd(ssl);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        if (fd >= 0) close(fd);
    }
    return 0;
}