/*
 Chain-of-Through process (high-level):
 1) Implement IPv6 TCP SSL client using OpenSSL in C.
 2) Enforce certificate validation, hostname verification, SNI, min TLS.
 3) Safe socket setup with timeouts and cleanup on error.
 4) Review memory paths and error handling.
 5) Final secure function and test harness.
*/

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void openssl_init_once_c() {
    static int inited = 0;
    if (!inited) {
        OPENSSL_init_ssl(0, NULL);
        inited = 1;
    }
}

static int connect_ipv6_tcp_c(const char* host, int port) {
    struct addrinfo hints;
    struct addrinfo* res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%d", port);

    int rc = getaddrinfo(host, portStr, &hints, &res);
    if (rc != 0) {
        return -1;
    }

    int fd = -1;
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;

        int v6only = 1;
        setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

        int yes = 1;
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));

        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
            freeaddrinfo(res);
            return fd;
        }
        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);
    return -1;
}

SSL* create_ipv6_ssl_client(const char* host, int port) {
    if (!host || !*host || port <= 0 || port > 65535) return NULL;

    openssl_init_once_c();

    int fd = connect_ipv6_tcp_c(host, port);
    if (fd < 0) return NULL;

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        close(fd);
        return NULL;
    }

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_default_verify_paths(ctx);

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        SSL_CTX_free(ctx);
        close(fd);
        return NULL;
    }

    // SNI
    SSL_set_tlsext_host_name(ssl, host);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    // Hostname verification (OpenSSL >= 1.1.0)
    SSL_set1_host(ssl, host);
#endif

    if (SSL_set_fd(ssl, fd) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);
        return NULL;
    }

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);
        return NULL;
    }

    SSL_CTX_free(ctx); // SSL holds a ref
    return ssl;
}

static void close_ssl_c(SSL* ssl) {
    if (!ssl) return;
    int fd = SSL_get_fd(ssl);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    if (fd >= 0) close(fd);
}

int main(void) {
    const char* hosts[5] = {
        "google.com",
        "cloudflare.com",
        "example.com",
        "ietf.org",
        "wikipedia.org"
    };
    int port = 443;

    for (int i = 0; i < 5; ++i) {
        const char* host = hosts[i];
        printf("Connecting to %s over IPv6 TLS...\n", host);
        SSL* ssl = create_ipv6_ssl_client(host, port);
        if (!ssl) {
            printf("Failed to connect to %s\n", host);
        } else {
            const char* ver = SSL_get_version(ssl);
            const char* cipher = SSL_get_cipher(ssl);
            printf("Connected. TLS: %s, Cipher: %s\n",
                   ver ? ver : "unknown",
                   cipher ? cipher : "unknown");
        }
        close_ssl_c(ssl);
        printf("----\n");
    }
    return 0;
}