#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static int send_all(int sock, const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(sock, data + total, len - total, 0);
        if (sent <= 0) return 0;
        total += (size_t)sent;
    }
    return 1;
}

static int parse_status_code(const char* response) {
    const char* p = strstr(response, "HTTP/");
    if (!p) return -1;
    p = strchr(p, ' ');
    if (!p) return -1;
    int code = atoi(p + 1);
    return code;
}

int upload_pdf(const char* host, const char* port, const char* path, const char* file_path) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "File not found: %s\n", file_path);
        return -1;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        fprintf(stderr, "Failed to seek file: %s\n", file_path);
        return -1;
    }
    long file_size = ftell(fp);
    if (file_size < 0) {
        fclose(fp);
        fprintf(stderr, "Failed to get file size: %s\n", file_path);
        return -1;
    }
    fseek(fp, 0, SEEK_SET);

    char boundary[64];
    snprintf(boundary, sizeof(boundary), "----LSE65Boundary%ld", (long)time(NULL));
    const char* CRLF = "\r\n";

    // Extract filename
    const char* filename = file_path;
    for (const char* p = file_path; *p; ++p) {
        if (*p == '/' || *p == '\\') filename = p + 1;
    }

    // Build pre-body
    const char* pre_fmt =
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"destination\"\r\n"
        "\r\n"
        "uploads\r\n"
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
        "Content-Type: application/pdf\r\n"
        "\r\n";

    int pre_len = snprintf(NULL, 0, pre_fmt, boundary, boundary, filename);
    char* pre_body = (char*)malloc((size_t)pre_len + 1);
    if (!pre_body) {
        fclose(fp);
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    snprintf(pre_body, (size_t)pre_len + 1, pre_fmt, boundary, boundary, filename);

    // Build post-body
    const char* post_fmt = "\r\n--%s--\r\n";
    int post_len = snprintf(NULL, 0, post_fmt, boundary);
    char* post_body = (char*)malloc((size_t)post_len + 1);
    if (!post_body) {
        fclose(fp);
        free(pre_body);
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    snprintf(post_body, (size_t)post_len + 1, post_fmt, boundary);

    long long content_length = (long long)pre_len + (long long)file_size + (long long)post_len;

    // Build header
    const char* hdr_fmt =
        "POST %s HTTP/1.1\r\n"
        "Host: %s:%s\r\n"
        "Content-Type: multipart/form-data; boundary=%s\r\n"
        "Content-Length: %lld\r\n"
        "Connection: close\r\n"
        "\r\n";

    int hdr_len = snprintf(NULL, 0, hdr_fmt, path, host, port, boundary, content_length);
    char* header = (char*)malloc((size_t)hdr_len + 1);
    if (!header) {
        fclose(fp);
        free(pre_body);
        free(post_body);
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }
    snprintf(header, (size_t)hdr_len + 1, hdr_fmt, path, host, port, boundary, content_length);

    // Connect
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* res = NULL;
    int gai = getaddrinfo(host, port, &hints, &res);
    if (gai != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai));
        fclose(fp);
        free(pre_body);
        free(post_body);
        free(header);
        return -1;
    }

    int sock = -1;
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) continue;
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);
    if (sock == -1) {
        fprintf(stderr, "Failed to connect to %s:%s\n", host, port);
        fclose(fp);
        free(pre_body);
        free(post_body);
        free(header);
        return -1;
    }

    // Send header and pre-body
    if (!send_all(sock, header, (size_t)hdr_len) ||
        !send_all(sock, pre_body, (size_t)pre_len)) {
        fprintf(stderr, "Failed to send header/pre-body\n");
        close(sock);
        fclose(fp);
        free(pre_body);
        free(post_body);
        free(header);
        return -1;
    }

    // Send file content
    char buf[8192];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (!send_all(sock, buf, r)) {
            fprintf(stderr, "Failed to send file data\n");
            close(sock);
            fclose(fp);
            free(pre_body);
            free(post_body);
            free(header);
            return -1;
        }
    }

    // Send post-body
    if (!send_all(sock, post_body, (size_t)post_len)) {
        fprintf(stderr, "Failed to send post-body\n");
        close(sock);
        fclose(fp);
        free(pre_body);
        free(post_body);
        free(header);
        return -1;
    }

    // Read response
    char rbuf[4096];
    size_t resp_cap = 8192;
    size_t resp_len = 0;
    char* response = (char*)malloc(resp_cap);
    if (!response) {
        fprintf(stderr, "Memory allocation failed\n");
        close(sock);
        fclose(fp);
        free(pre_body);
        free(post_body);
        free(header);
        return -1;
    }

    ssize_t n;
    while ((n = recv(sock, rbuf, sizeof(rbuf), 0)) > 0) {
        if (resp_len + (size_t)n >= resp_cap) {
            resp_cap *= 2;
            char* tmp = (char*)realloc(response, resp_cap);
            if (!tmp) {
                fprintf(stderr, "Memory allocation failed\n");
                free(response);
                close(sock);
                fclose(fp);
                free(pre_body);
                free(post_body);
                free(header);
                return -1;
            }
            response = tmp;
        }
        memcpy(response + resp_len, rbuf, (size_t)n);
        resp_len += (size_t)n;
    }
    // Null-terminate for parsing convenience
    if (resp_len >= resp_cap) {
        char* tmp = (char*)realloc(response, resp_cap + 1);
        if (tmp) {
            response = tmp;
            resp_cap += 1;
        }
    }
    response[resp_len] = '\0';

    int status = parse_status_code(response);

    // Cleanup
    free(response);
    close(sock);
    fclose(fp);
    free(pre_body);
    free(post_body);
    free(header);

    return status;
}

int main(void) {
    const char* host = "127.0.0.1";
    const char* port = "8080";
    const char* path = "/upload";

    const char* files[5] = {
        "./sample1.pdf",
        "./sample2.pdf",
        "./docs/test.pdf",
        "./data/report.pdf",
        "./dummy.pdf"
    };

    for (int i = 0; i < 5; ++i) {
        int status = upload_pdf(host, port, path, files[i]);
        printf("Upload: %s -> HTTP Status: %d\n", files[i], status);
    }

    return 0;
}