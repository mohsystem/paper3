#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>

static SSL_CTX* get_global_ctx() {
    static SSL_CTX* ctx = NULL;
    static int initialized = 0;
    if (!initialized) {
        initialized = 1;
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

SSL* create_ipv6_ssl_client(const char* host, int port) {
    SSL_CTX* ctx = get_global_ctx();
    if (!ctx) return NULL;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* res = NULL;
    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    int gai = getaddrinfo(host, portstr, &hints, &res);
    if (gai != 0) {
        return NULL;
    }

    int fd = -1;
    struct addrinfo* ai;
    for (ai = res; ai != NULL; ai = ai->ai_next) {
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
        return NULL;
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        close(fd);
        return NULL;
    }

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
    SSL_set_tlsext_host_name(ssl, host);
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    SSL_set1_host(ssl, host);
#endif
    SSL_set_verify(ssl, SSL_VERIFY_PEER, NULL);
    SSL_set_fd(ssl, fd);

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        close(fd);
        return NULL;
    }

    return ssl;
}

int main() {
    const char* hosts[5] = {
        "google.com",
        "cloudflare.com",
        "example.com",
        "wikipedia.org",
        "ietf.org"
    };

    for (int i = 0; i < 5; ++i) {
        SSL* ssl = create_ipv6_ssl_client(hosts[i], 443);
        if (ssl) {
            const SSL_CIPHER* cipher = SSL_get_current_cipher(ssl);
            printf("Connected to %s via IPv6 using %s %s\n",
                   hosts[i],
                   SSL_get_version(ssl),
                   cipher ? SSL_CIPHER_get_name(cipher) : "unknown");
            int fd = SSL_get_fd(ssl);
            SSL_free(ssl);
            if (fd >= 0) close(fd);
        } else {
            printf("Failed to connect to %s\n", hosts[i]);
        }
    }

    return 0;
}