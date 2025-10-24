#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

static char* str_dup_s(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

static char* format_str(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (len < 0) { va_end(ap); return NULL; }
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) { va_end(ap); return NULL; }
    vsnprintf(buf, (size_t)len + 1, fmt, ap);
    va_end(ap);
    return buf;
}

char* establish_secure_connection(const char* host, int port, int timeout_ms) {
    if (!host || strlen(host) == 0) return str_dup_s("ERROR: Invalid host");
    if (port <= 0 || port > 65535) return str_dup_s("ERROR: Invalid port");
    if (timeout_ms <= 0) return str_dup_s("ERROR: Invalid timeout");

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        return str_dup_s("ERROR: SSL_CTX_new failed");
    }

    char* result = NULL;

    do {
        if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) != 1) {
            result = str_dup_s("ERROR: Failed to set min TLS version");
            break;
        }

        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
        if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
            result = str_dup_s("ERROR: Failed to load default CA paths");
            break;
        }

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_UNSPEC;

        struct addrinfo* res = NULL;
        char portstr[16];
        snprintf(portstr, sizeof(portstr), "%d", port);
        int gai = getaddrinfo(host, portstr, &hints, &res);
        if (gai != 0) {
            result = format_str("ERROR: getaddrinfo: %s", gai_strerror(gai));
            break;
        }

        int fd = -1;
        struct addrinfo* p;
        for (p = res; p != NULL; p = p->ai_next) {
            fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (fd < 0) continue;

            int flags = fcntl(fd, F_GETFL, 0);
            if (flags < 0) { close(fd); fd = -1; continue; }
            if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) { close(fd); fd = -1; continue; }

            int rc = connect(fd, p->ai_addr, p->ai_addrlen);
            if (rc < 0 && errno != EINPROGRESS) {
                close(fd); fd = -1; continue;
            }

            if (rc != 0) {
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET(fd, &wfds);
                struct timeval tv;
                tv.tv_sec = timeout_ms / 1000;
                tv.tv_usec = (timeout_ms % 1000) * 1000;

                rc = select(fd + 1, NULL, &wfds, NULL, &tv);
                if (rc <= 0) {
                    close(fd); fd = -1; continue;
                }
                int so_error = 0;
                socklen_t slen = sizeof(so_error);
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &slen) < 0 || so_error != 0) {
                    close(fd); fd = -1; continue;
                }
            }

            // Back to blocking
            fcntl(fd, F_SETFL, flags);

            struct timeval rcv;
            rcv.tv_sec = timeout_ms / 1000;
            rcv.tv_usec = (timeout_ms % 1000) * 1000;
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv, sizeof(rcv));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &rcv, sizeof(rcv));

            SSL* ssl = SSL_new(ctx);
            if (!ssl) { close(fd); fd = -1; continue; }

            if (SSL_set_tlsext_host_name(ssl, host) != 1) {
                SSL_free(ssl); close(fd); fd = -1; continue;
            }
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
            if (SSL_set1_host(ssl, host) != 1) {
                SSL_free(ssl); close(fd); fd = -1; continue;
            }
#else
            // Older OpenSSL lacks SSL_set1_host; would require manual hostname verification.
            // For security, we abort if not available.
            SSL_free(ssl); close(fd); fd = -1;
            result = str_dup_s("ERROR: Hostname verification not supported on this OpenSSL version");
            break;
#endif

            if (SSL_set_fd(ssl, fd) != 1) {
                SSL_free(ssl); close(fd); fd = -1; continue;
            }

            int ok = SSL_connect(ssl);
            if (ok != 1) {
                SSL_free(ssl); close(fd); fd = -1; continue;
            }

            const char* proto = SSL_get_version(ssl);
            const char* cipher = SSL_get_cipher_name(ssl);

            char* subject = NULL;
            X509* cert = SSL_get_peer_certificate(ssl);
            if (cert) {
                char* subj = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
                if (subj) {
                    subject = str_dup_s(subj);
                    OPENSSL_free(subj);
                }
                X509_free(cert);
            }
            if (!subject) subject = str_dup_s("");

            result = format_str("OK host=%s port=%d protocol=%s cipher=%s subject=\"%s\"",
                                host, port, proto ? proto : "unknown", cipher ? cipher : "unknown", subject);
            free(subject);

            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(fd);
            break;
        }

        freeaddrinfo(res);
        if (!result) {
            if (!result) result = str_dup_s("ERROR: Connection/Handshake failed");
        }
    } while (0);

    EVP_cleanup();
    ERR_free_strings();

    return result;
}

int main(void) {
    const char* hosts[5] = {
        "www.google.com",
        "www.cloudflare.com",
        "www.github.com",
        "www.wikipedia.org",
        "api.openai.com"
    };
    int port = 443;
    int timeout_ms = 8000;

    for (int i = 0; i < 5; ++i) {
        char* res = establish_secure_connection(hosts[i], port, timeout_ms);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("ERROR: Unknown failure\n");
        }
    }
    return 0;
}