#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static int connect_with_timeout_c(const char* host, int port, int timeout_sec) {
    struct addrinfo hints, *res = NULL, *p = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);

    if (getaddrinfo(host, portstr, &hints, &res) != 0) {
        return -1;
    }

    int sockfd = -1;
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) continue;

        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        int rc = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (rc < 0 && errno == EINPROGRESS) {
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(sockfd, &wfds);
            struct timeval tv;
            tv.tv_sec = timeout_sec;
            tv.tv_usec = 0;
            rc = select(sockfd + 1, NULL, &wfds, NULL, &tv);
            if (rc > 0) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0) {
                    fcntl(sockfd, F_SETFL, flags);
                    break;
                } else {
                    close(sockfd);
                    sockfd = -1;
                    continue;
                }
            } else {
                close(sockfd);
                sockfd = -1;
                continue;
            }
        } else if (rc == 0) {
            fcntl(sockfd, F_SETFL, flags);
            break;
        } else {
            close(sockfd);
            sockfd = -1;
            continue;
        }
    }
    freeaddrinfo(res);
    return sockfd;
}

char* connect_tls(const char* host, int port, int timeout_seconds) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        char* r = (char*)malloc(64);
        snprintf(r, 64, "ERROR: SSL_CTX_new failed");
        return r;
    }
    SSL_CTX_set_default_verify_paths(ctx);

    SSL* ssl = NULL;
    int sock = -1;
    char* result = NULL;

    do {
        sock = connect_with_timeout_c(host, port, timeout_seconds);
        if (sock < 0) {
            result = (char*)malloc(64);
            snprintf(result, 64, "ERROR: TCP connect failed");
            break;
        }

        ssl = SSL_new(ctx);
        if (!ssl) {
            result = (char*)malloc(64);
            snprintf(result, 64, "ERROR: SSL_new failed");
            break;
        }

        SSL_set_tlsext_host_name(ssl, host);
        X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
        X509_VERIFY_PARAM_set_hostflags(param, 0);
        X509_VERIFY_PARAM_set1_host(param, host, 0);
        SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);
        SSL_set_fd(ssl, sock);

        if (SSL_connect(ssl) != 1) {
            unsigned long e = ERR_get_error();
            const char* emsg = e ? ERR_error_string(e, NULL) : "unknown";
            size_t len = strlen(emsg) + 32;
            result = (char*)malloc(len);
            snprintf(result, len, "ERROR: SSL_connect failed: %s", emsg);
            break;
        }

        const char* cipher = SSL_get_cipher(ssl);
        const char* proto = SSL_get_version(ssl);

        X509* cert = SSL_get_peer_certificate(ssl);
        char subj[1024] = "unverified";
        char issr[1024] = "unverified";
        if (cert) {
            X509_NAME_oneline(X509_get_subject_name(cert), subj, sizeof(subj));
            X509_NAME_oneline(X509_get_issuer_name(cert), issr, sizeof(issr));
            X509_free(cert);
        }

        // Build result string
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "OK %s:%d protocol=%s cipher=%s subject=%s issuer=%s",
                 host, port, proto ? proto : "", cipher ? cipher : "", subj, issr);
        result = (char*)malloc(strlen(buffer) + 1);
        strcpy(result, buffer);

    } while (0);

    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock >= 0) close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return result;
}

int main(void) {
    const char* hosts[5] = {
        "www.google.com",
        "www.cloudflare.com",
        "www.github.com",
        "www.stackoverflow.com",
        "expired.badssl.com"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = connect_tls(hosts[i], 443, 8);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("ERROR: unknown\n");
        }
    }
    return 0;
}