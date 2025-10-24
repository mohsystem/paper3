#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>

static bool is_ip_address(const char* host) {
    struct in_addr a4;
    struct in6_addr a6;
    if (inet_pton(AF_INET, host, &a4) == 1) return true;
    if (inet_pton(AF_INET6, host, &a6) == 1) return true;
    return false;
}

static int set_blocking(int fd, int blocking) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    if (blocking) flags &= ~O_NONBLOCK;
    else flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

static int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, int timeout_ms) {
    if (set_blocking(fd, 0) != 0) return -1;
    int rc = connect(fd, addr, addrlen);
    if (rc == 0) {
        set_blocking(fd, 1);
        return 0;
    }
    if (rc < 0 && errno != EINPROGRESS) {
        set_blocking(fd, 1);
        return -1;
    }
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    rc = select(fd + 1, NULL, &wfds, NULL, &tv);
    if (rc <= 0) {
        set_blocking(fd, 1);
        errno = (rc == 0) ? ETIMEDOUT : errno;
        return -1;
    }
    int so_error = 0;
    socklen_t len = sizeof(so_error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
        set_blocking(fd, 1);
        return -1;
    }
    set_blocking(fd, 1);
    if (so_error != 0) {
        errno = so_error;
        return -1;
    }
    return 0;
}

static SSL_CTX* create_ctx(void) {
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) return NULL;
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_ciphersuites(ctx, "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_128_GCM_SHA256");
    if (SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4") != 1) {
        // Fallback to defaults if not supported
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_default_verify_paths(ctx);
    return ctx;
}

SSL* create_ipv6_tls_client(const char* host, int port, int timeout_ms, int* out_fd) {
    if (!out_fd) {
        errno = EINVAL;
        return NULL;
    }
    *out_fd = -1;
    if (!host || !*host || port < 1 || port > 65535 || timeout_ms <= 0 || timeout_ms > 60000) {
        errno = EINVAL;
        return NULL;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* res = NULL;
    int gai = getaddrinfo(host, NULL, &hints, &res);
    if (gai != 0 || !res) {
        if (res) freeaddrinfo(res);
        errno = EHOSTUNREACH;
        return NULL;
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons((unsigned short)port);
    bool found = false;
    for (struct addrinfo* p = res; p; p = p->ai_next) {
        if (p->ai_family == AF_INET6 && p->ai_addrlen >= sizeof(struct sockaddr_in6)) {
            memcpy(&addr.sin6_addr, &((struct sockaddr_in6*)p->ai_addr)->sin6_addr, sizeof(struct in6_addr));
            addr.sin6_scope_id = ((struct sockaddr_in6*)p->ai_addr)->sin6_scope_id;
            found = true;
            break;
        }
    }
    freeaddrinfo(res);
    if (!found) {
        errno = EHOSTUNREACH;
        return NULL;
    }

    int fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) return NULL;

    if (connect_with_timeout(fd, (struct sockaddr*)&addr, sizeof(addr), timeout_ms) != 0) {
        close(fd);
        return NULL;
    }

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    SSL_CTX* ctx = create_ctx();
    if (!ctx) {
        close(fd);
        return NULL;
    }

    SSL* ssl = SSL_new(ctx);
    SSL_CTX_free(ctx);
    if (!ssl) {
        close(fd);
        return NULL;
    }

    X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (is_ip_address(host)) {
        if (X509_VERIFY_PARAM_set1_ip_asc(param, host) != 1) {
            SSL_free(ssl);
            close(fd);
            errno = EINVAL;
            return NULL;
        }
    } else {
        if (X509_VERIFY_PARAM_set1_host(param, host, 0) != 1) {
            SSL_free(ssl);
            close(fd);
            errno = EINVAL;
            return NULL;
        }
        SSL_set_tlsext_host_name(ssl, host);
    }

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

    *out_fd = fd;
    return ssl;
}

void close_ipv6_tls_client(SSL* ssl, int fd) {
    if (ssl) {
        int r = SSL_shutdown(ssl);
        if (r == 0) {
            SSL_shutdown(ssl);
        }
        SSL_free(ssl);
    }
    if (fd >= 0) {
        close(fd);
    }
}

int main(void) {
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    const char* hosts[5] = {
        "google.com",
        "cloudflare.com",
        "wikipedia.org",
        "ietf.org",
        "facebook.com"
    };

    for (int i = 0; i < 5; ++i) {
        const char* h = hosts[i];
        int fd = -1;
        SSL* ssl = NULL;
        printf("Connecting (IPv6/TLS) to %s:443\n", h);
        ssl = create_ipv6_tls_client(h, 443, 4000, &fd);
        if (ssl) {
            const SSL_CIPHER* c = SSL_get_current_cipher(ssl);
            printf("Success: %s - Protocol=%s, Cipher=%s\n",
                   h, SSL_get_version(ssl), c ? SSL_CIPHER_get_name(c) : "unknown");
            close_ipv6_tls_client(ssl, fd);
        } else {
            unsigned long err = ERR_get_error();
            char buf[256];
            if (err) {
                ERR_error_string_n(err, buf, sizeof(buf));
                printf("Failed: %s - %s\n", h, buf);
            } else {
                snprintf(buf, sizeof(buf), "%s", strerror(errno));
                printf("Failed: %s - %s\n", h, buf);
            }
            if (fd >= 0) close(fd);
        }
    }

    EVP_cleanup();
    ERR_free_strings();
    return 0;
}