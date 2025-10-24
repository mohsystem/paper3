#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static char* percent_encode_segment(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(s);
    // Worst case every char encoded to %XX => 3x
    char* out = (char*)malloc(len * 3 + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out[j++] = (char)c;
        } else {
            static const char* hex = "0123456789ABCDEF";
            out[j++] = '%';
            out[j++] = hex[(c >> 4) & 0xF];
            out[j++] = hex[c & 0xF];
        }
    }
    out[j] = '\0';
    return out;
}

static char* build_path(const char* prefix, const char* filename) {
    char* enc = percent_encode_segment(filename ? filename : "");
    if (!enc) return NULL;
    size_t plen = prefix ? strlen(prefix) : 0;
    // normalize prefix
    char* pnorm = NULL;
    if (!prefix || plen == 0) {
        pnorm = strdup("/");
    } else {
        // ensure leading slash
        if (prefix[0] != '/') {
            pnorm = (char*)malloc(plen + 2);
            if (!pnorm) { free(enc); return NULL; }
            pnorm[0] = '/';
            memcpy(pnorm + 1, prefix, plen);
            pnorm[plen + 1] = '\0';
        } else {
            pnorm = strdup(prefix);
        }
    }
    if (!pnorm) { free(enc); return NULL; }

    // If enc is empty
    if (enc[0] == '\0') {
        if (strcmp(pnorm, "/") != 0) {
            // return pnorm
            free(enc);
            return pnorm;
        } else {
            free(enc);
            return strdup("/");
        }
    }

    int slash = (pnorm[strlen(pnorm) - 1] == '/');
    size_t total = strlen(pnorm) + (slash ? 0 : 1) + strlen(enc) + 1;
    char* path = (char*)malloc(total);
    if (!path) { free(enc); free(pnorm); return NULL; }
    strcpy(path, pnorm);
    if (!slash) strcat(path, "/");
    strcat(path, enc);
    free(enc);
    free(pnorm);
    return path;
}

static int has_chunked_te(const char* headers) {
    char* lower = strdup(headers);
    if (!lower) return 0;
    for (char* p = lower; *p; ++p) *p = (char)tolower((unsigned char)*p);
    int found = strstr(lower, "transfer-encoding: chunked") != NULL;
    free(lower);
    return found;
}

static char* decode_chunked(const char* body, size_t body_len, size_t* out_len) {
    size_t i = 0;
    size_t cap = body_len;
    char* out = (char*)malloc(cap + 1);
    if (!out) return NULL;
    size_t j = 0;

    while (i < body_len) {
        // find line end
        size_t line_start = i;
        while (i + 1 < body_len && !(body[i] == '\r' && body[i+1] == '\n')) i++;
        if (i + 1 >= body_len) break;
        size_t line_len = i - line_start;
        char* size_str = (char*)malloc(line_len + 1);
        if (!size_str) { free(out); return NULL; }
        memcpy(size_str, body + line_start, line_len);
        size_str[line_len] = '\0';
        // strip extensions
        char* semi = strchr(size_str, ';');
        if (semi) *semi = '\0';
        // parse hex
        unsigned long chunk_size = strtoul(size_str, NULL, 16);
        free(size_str);
        i += 2; // skip CRLF
        if (chunk_size == 0) {
            // terminator, skip optional trailer (not processed)
            break;
        }
        if (i + chunk_size > body_len) break;
        if (j + chunk_size + 1 > cap) {
            cap = (j + chunk_size + 1) * 2;
            char* tmp = (char*)realloc(out, cap);
            if (!tmp) { free(out); return NULL; }
            out = tmp;
        }
        memcpy(out + j, body + i, chunk_size);
        j += chunk_size;
        i += chunk_size;
        // skip CRLF after chunk
        if (i + 1 < body_len && body[i] == '\r' && body[i+1] == '\n') i += 2;
        else break;
    }
    out[j] = '\0';
    if (out_len) *out_len = j;
    return out;
}

char* fetch_file(const char* host, const char* path_prefix, const char* file_name, int port) {
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = NULL;
    int gai = getaddrinfo(host, port_str, &hints, &res);
    if (gai != 0) {
        char* err = (char*)malloc(256);
        snprintf(err, 256, "DNS error: %s", gai_strerror(gai));
        return err;
    }

    int sockfd = -1;
    for (struct addrinfo* p = res; p; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);
    if (sockfd == -1) {
        char* err = strdup("Connection error");
        return err ? err : NULL;
    }

    char* path = build_path(path_prefix, file_name ? file_name : "");
    if (!path) {
        close(sockfd);
        return strdup("Path build error");
    }

    char req[2048];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Task129/1.0\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n",
             path, host);

    free(path);

    size_t to_send = strlen(req);
    size_t sent = 0;
    while (sent < to_send) {
        ssize_t n = send(sockfd, req + sent, to_send - sent, 0);
        if (n <= 0) { close(sockfd); return strdup("Send error"); }
        sent += (size_t)n;
    }

    size_t cap = 8192;
    size_t len = 0;
    char* resp = (char*)malloc(cap);
    if (!resp) { close(sockfd); return strdup("Alloc error"); }

    for (;;) {
        if (len + 4096 > cap) {
            cap *= 2;
            char* tmp = (char*)realloc(resp, cap);
            if (!tmp) { free(resp); close(sockfd); return strdup("Alloc error"); }
            resp = tmp;
        }
        ssize_t n = recv(sockfd, resp + len, cap - len, 0);
        if (n <= 0) break;
        len += (size_t)n;
    }
    close(sockfd);
    resp[len] = '\0';

    // find header-body separator
    char* sep = strstr(resp, "\r\n\r\n");
    if (!sep) {
        return resp; // return full response
    }
    size_t header_len = (size_t)(sep - resp);
    char* headers = (char*)malloc(header_len + 1);
    if (!headers) { return resp; }
    memcpy(headers, resp, header_len);
    headers[header_len] = '\0';

    size_t body_len = len - header_len - 4;
    char* body = (char*)malloc(body_len + 1);
    if (!body) { free(headers); return resp; }
    memcpy(body, sep + 4, body_len);
    body[body_len] = '\0';

    int chunked = has_chunked_te(headers);
    free(headers);
    free(resp);

    if (chunked) {
        size_t out_len = 0;
        char* decoded = decode_chunked(body, body_len, &out_len);
        free(body);
        if (decoded) return decoded;
        return strdup("Chunked decode error");
    } else {
        return body;
    }
}

int main() {
    const char* host = "example.com";
    const char* prefix = "/";
    const char* files[5] = { "", "robots.txt", "favicon.ico", "nonexisting.file", "index.html" };
    for (int i = 0; i < 5; ++i) {
        char* content = fetch_file(host, prefix, files[i], 80);
        if (!content) {
            printf("Test %d (%s): error\n-----\n", i + 1, files[i]);
            continue;
        }
        size_t len = strlen(content);
        printf("Test %d (%s): length=%zu\n", i + 1, files[i], len);
        size_t preview = len < 200 ? len : 200;
        fwrite(content, 1, preview, stdout);
        printf("\n-----\n");
        free(content);
    }
    return 0;
}