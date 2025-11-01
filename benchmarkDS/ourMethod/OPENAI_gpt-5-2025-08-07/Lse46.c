#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE (5 * 1024 * 1024)

static bool is_valid_char(char c) {
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '.' || c == '_' || c == '-') return true;
    return false;
}

static bool is_valid_name(const char* name) {
    if (name == NULL) return false;
    size_t len = strlen(name);
    if (len == 0 || len > 100) return false;
    for (size_t i = 0; i < len; i++) {
        if (!is_valid_char(name[i])) return false;
    }
    if (strstr(name, "..") != NULL) return false;
    if (strchr(name, '/') != NULL || strchr(name, '\\') != NULL) return false;
    return true;
}

// Securely open and read images/name. Returns 0 on success.
// On success, *out_buf allocated and must be free()'d by caller. *out_len is length.
// Errors: -2 invalid, -1 not found, -3 too large/IO error.
int open_image(const char* name, unsigned char** out_buf, size_t* out_len) {
    *out_buf = NULL;
    *out_len = 0;

    if (!is_valid_name(name)) return -2;

    int dirfd = open("images", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return -1;

    int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = openat(dirfd, name, flags);
    if (fd < 0) {
        close(dirfd);
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        close(dirfd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        close(dirfd);
        return -1;
    }
    if (st.st_size < 0 || (size_t)st.st_size > MAX_SIZE) {
        close(fd);
        close(dirfd);
        return -3;
    }

    size_t size = (size_t)st.st_size;
    unsigned char* buf = NULL;
    if (size > 0) {
        buf = (unsigned char*)malloc(size);
        if (buf == NULL) {
            close(fd);
            close(dirfd);
            return -3;
        }
        size_t off = 0;
        while (off < size) {
            ssize_t r = read(fd, buf + off, size - off);
            if (r < 0) {
                free(buf);
                close(fd);
                close(dirfd);
                return -3;
            }
            if (r == 0) break;
            off += (size_t)r;
        }
        *out_len = off;
    } else {
        buf = NULL;
        *out_len = 0;
    }

    *out_buf = buf;
    close(fd);
    close(dirfd);
    return 0;
}

// Minimal HTTP-like response builder for /img?name=...
char* handle_img_request(const char* name_param, size_t* resp_len) {
    unsigned char* data = NULL;
    size_t data_len = 0;
    int rc = open_image(name_param, &data, &data_len);
    const char* header_400 = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
    const char* header_404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    const char* header_500 = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";

    if (rc == -2) {
        *resp_len = strlen(header_400);
        char* resp = (char*)malloc(*resp_len + 1);
        if (resp) memcpy(resp, header_400, *resp_len + 1);
        return resp;
    }
    if (rc == -1) {
        *resp_len = strlen(header_404);
        char* resp = (char*)malloc(*resp_len + 1);
        if (resp) memcpy(resp, header_404, *resp_len + 1);
        return resp;
    }
    if (rc != 0) {
        *resp_len = strlen(header_500);
        char* resp = (char*)malloc(*resp_len + 1);
        if (resp) memcpy(resp, header_500, *resp_len + 1);
        return resp;
    }

    char header[256];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %zu\r\n\r\n",
                     data_len);
    if (n < 0 || (size_t)n >= sizeof(header)) {
        free(data);
        *resp_len = strlen(header_500);
        char* resp = (char*)malloc(*resp_len + 1);
        if (resp) memcpy(resp, header_500, *resp_len + 1);
        return resp;
    }
    size_t header_len = (size_t)n;
    *resp_len = header_len + data_len;
    char* resp = (char*)malloc(*resp_len);
    if (!resp) {
        free(data);
        *resp_len = strlen(header_500);
        resp = (char*)malloc(*resp_len + 1);
        if (resp) memcpy(resp, header_500, *resp_len + 1);
        return resp;
    }
    memcpy(resp, header, header_len);
    if (data_len > 0) memcpy(resp + header_len, data, data_len);
    free(data);
    return resp;
}

int main(void) {
    const char* tests[5] = {"test.png", "a.txt", "../etc/passwd", "nonexistent.xxx", "valid-name.jpg"};
    for (int i = 0; i < 5; i++) {
        unsigned char* buf = NULL;
        size_t len = 0;
        int rc = open_image(tests[i], &buf, &len);
        if (rc == 0) {
            printf("Test [%s]: OK, size=%zu\n", tests[i], len);
            free(buf);
        } else if (rc == -1) {
            printf("Test [%s]: NotFound\n", tests[i]);
        } else if (rc == -2) {
            printf("Test [%s]: Rejected\n", tests[i]);
        } else {
            printf("Test [%s]: Error\n", tests[i]);
        }
    }
    // Example HTTP-like response:
    // size_t resp_len = 0;
    // char* resp = handle_img_request("test.png", &resp_len);
    // if (resp) { write(1, resp, resp_len); free(resp); }
    return 0;
}