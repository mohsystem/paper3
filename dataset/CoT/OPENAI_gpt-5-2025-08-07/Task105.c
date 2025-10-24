#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

static int connect_with_timeout_c(int sockfd, const struct sockaddr *addr, socklen_t addrlen, int timeout_ms) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;

    int ret = connect(sockfd, addr, addrlen);
    if (ret == 0) {
        fcntl(sockfd, F_SETFL, flags);
        return 0;
    }
    if (errno != EINPROGRESS) {
        fcntl(sockfd, F_SETFL, flags);
        return -1;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(sockfd + 1, NULL, &wfds, NULL, &tv);
    if (ret <= 0) {
        fcntl(sockfd, F_SETFL, flags);
        errno = (ret == 0) ? ETIMEDOUT : errno;
        return -1;
    }

    int so_error = 0;
    socklen_t len = sizeof(so_error);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
        fcntl(sockfd, F_SETFL, flags);
        return -1;
    }
    if (so_error != 0) {
        fcntl(sockfd, F_SETFL, flags);
        errno = so_error;
        return -1;
    }

    fcntl(sockfd, F_SETFL, flags);
    return 0;
}

static int dial_host_c(const char *host, int port, int timeout_ms) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    int rc = getaddrinfo(host, portstr, &hints, &res);
    if (rc != 0) return -1;

    int sockfd = -1;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        int s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s < 0) continue;

        if (connect_with_timeout_c(s, p->ai_addr, p->ai_addrlen, timeout_ms) == 0) {
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
            sockfd = s;
            break;
        }
        close(s);
    }
    freeaddrinfo(res);
    return sockfd;
}

// Caller owns the returned string and must free() it.
char* https_get(const char *host, int port, const char *path_in, int timeout_ms) {
    if (!host || !*host) {
        char *err = strdup("ERROR: host is required");
        return err;
    }
    if (port <= 0) port = 443;
    if (timeout_ms <= 0) timeout_ms = 10000;

    char pathbuf[1024];
    if (!path_in || !*path_in) {
        strcpy(pathbuf, "/");
    } else {
        if (path_in[0] == '/') {
            snprintf(pathbuf, sizeof(pathbuf), "%s", path_in);
        } else {
            snprintf(pathbuf, sizeof(pathbuf), "/%s", path_in);
        }
    }

    OPENSSL_init_ssl(0, NULL);
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return strdup("ERROR: SSL_CTX_new failed");
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        SSL_CTX_load_verify_locations(ctx, "/etc/ssl/certs/ca-certificates.crt", NULL);
    }

    X509_VERIFY_PARAM *param = SSL_CTX_get0_param(ctx);
    X509_VERIFY_PARAM_set_hostflags(param, 0);
    X509_VERIFY_PARAM_set1_host(param, host, 0);

    int sockfd = dial_host_c(host, port, timeout_ms);
    if (sockfd < 0) {
        SSL_CTX_free(ctx);
        char buf[256];
        snprintf(buf, sizeof(buf), "ERROR: connect failed: %s", strerror(errno));
        return strdup(buf);
    }

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        close(sockfd);
        SSL_CTX_free(ctx);
        return strdup("ERROR: SSL_new failed");
    }

    if (SSL_set_tlsext_host_name(ssl, host) != 1) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return strdup("ERROR: SNI setup failed");
    }

    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) != 1) {
        unsigned long err = ERR_get_error();
        char ebuf[256];
        ERR_error_string_n(err, ebuf, sizeof(ebuf));
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        char *msg;
        asprintf(&msg, "ERROR: SSL_connect failed: %s", ebuf);
        if (!msg) msg = strdup("ERROR: SSL_connect failed");
        return msg;
    }

    long verify = SSL_get_verify_result(ssl);
    if (verify != X509_V_OK) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return strdup("ERROR: certificate verify failed");
    }

    char req[2048];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Task105-C/1.0\r\n"
             "Accept: */*\r\n"
             "Connection: close\r\n\r\n",
             pathbuf, host);

    if (SSL_write(ssl, req, (int)strlen(req)) <= 0) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return strdup("ERROR: SSL_write failed");
    }

    // Read status line
    char status[8192];
    size_t idx = 0;
    int got_cr = 0;
    while (idx + 1 < sizeof(status)) {
        char ch;
        int n = SSL_read(ssl, &ch, 1);
        if (n <= 0) break;
        if (ch == '\r') {
            got_cr = 1;
            continue;
        }
        if (ch == '\n') break;
        if (got_cr) got_cr = 0;
        status[idx++] = ch;
    }
    status[idx] = '\0';
    if (idx == 0) {
        strcpy(status, "ERROR: No response received");
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);

    return strdup(status);
}

int main(void) {
    const char *tests[5][3] = {
        {"www.google.com", "443", "/"},
        {"www.github.com", "443", "/"},
        {"www.cloudflare.com", "443", "/"},
        {"www.wikipedia.org", "443", "/"},
        {"example.com", "443", "/"}
    };
    for (int i = 0; i < 5; ++i) {
        const char *host = tests[i][0];
        int port = atoi(tests[i][1]);
        const char *path = tests[i][2];
        char *res = https_get(host, port, path, 10000);
        if (res) {
            printf("%s: %s\n", host, res);
            free(res);
        } else {
            printf("%s: ERROR - unknown\n", host);
        }
    }
    return 0;
}