#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Function that accepts input as parameter and returns output (heap-allocated string; caller frees)
char* build_message(const char* input) {
    const char* in = input ? input : "";
    // trim
    while (*in && (*in == ' ' || *in == '\t' || *in == '\r' || *in == '\n')) in++;
    size_t len = strlen(in);
    while (len > 0 && (in[len - 1] == ' ' || in[len - 1] == '\t' || in[len - 1] == '\r' || in[len - 1] == '\n')) len--;

    char name[64];
    if (len == 0) {
        snprintf(name, sizeof(name), "%s", "World");
    } else {
        if (len > 50) {
            return NULL; // invalid length
        }
        // validate characters
        for (size_t i = 0; i < len; i++) {
            unsigned char ch = (unsigned char)in[i];
            if (!(isalnum(ch) || ch == ' ' || ch == '_' || ch == '-')) {
                return NULL; // invalid char
            }
        }
        memcpy(name, in, len);
        name[len] = '\0';
    }

    const char* prefix = "Hello, ";
    const char* suffix = "!";
    size_t outlen = strlen(prefix) + strlen(name) + strlen(suffix);
    char* out = (char*)malloc(outlen + 1);
    if (!out) return NULL;
    snprintf(out, outlen + 1, "%s%s%s", prefix, name, suffix);
    return out;
}

static void url_decode_inplace(char* s) {
    size_t r = 0, w = 0;
    while (s[r]) {
        if (s[r] == '+') {
            s[w++] = ' ';
            r++;
        } else if (s[r] == '%' && s[r + 1] && s[r + 2]) {
            char hex[3] = { s[r + 1], s[r + 2], '\0' };
            char* endp = NULL;
            long v = strtol(hex, &endp, 16);
            if (endp && *endp == '\0' && v >= 0 && v <= 255) {
                s[w++] = (char)v;
                r += 3;
            } else {
                s[w++] = s[r++];
            }
        } else {
            s[w++] = s[r++];
        }
    }
    s[w] = '\0';
}

static char* get_name_from_request_line(const char* reqline) {
    // Copy to buffer to safely modify for parsing
    const char* sp1 = strchr(reqline, ' ');
    if (!sp1) return NULL;
    const char* sp2 = strchr(sp1 + 1, ' ');
    if (!sp2) return NULL;
    size_t pathlen = (size_t)(sp2 - (sp1 + 1));
    if (pathlen == 0 || pathlen > 1024) return NULL;
    char* path = (char*)malloc(pathlen + 1);
    if (!path) return NULL;
    memcpy(path, sp1 + 1, pathlen);
    path[pathlen] = '\0';

    char* q = strchr(path, '?');
    if (!q) { free(path); return NULL; }
    char* query = q + 1;

    char* saved = NULL;
    char* tok = strtok_r(query, "&", &saved);
    char* result = NULL;
    while (tok) {
        char* eq = strchr(tok, '=');
        if (eq) {
            *eq = '\0';
            char* key = tok;
            char* val = eq + 1;
            if (strcmp(key, "name") == 0) {
                url_decode_inplace(val);
                result = strdup(val);
                break;
            }
        }
        tok = strtok_r(NULL, "&", &saved);
    }
    free(path);
    return result;
}

static volatile sig_atomic_t stop_flag_c = 0;
static void handle_sigint_c(int sig) { (void)sig; stop_flag_c = 1; }

int main(void) {
    // 5 test cases
    const char* tests[5] = {"World", "Alice", "Bob_12", "Eve-Ok", " "};
    for (int i = 0; i < 5; i++) {
        char* msg = build_message(tests[i]);
        if (msg) {
            printf("%s\n", msg);
            free(msg);
        } else {
            printf("Error: Invalid input\n");
        }
    }

    // Minimal HTTP server on localhost:8083
    signal(SIGINT, handle_sigint_c);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(fd);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8083);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(fd);
        return 1;
    }

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return 1;
    }
    if (listen(fd, 16) < 0) {
        perror("listen");
        close(fd);
        return 1;
    }

    while (!stop_flag_c) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EINTR && stop_flag_c) break;
            perror("accept");
            continue;
        }
        char buf[1024];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(cfd);
            continue;
        }
        buf[n] = '\0';
        char* name = get_name_from_request_line(buf);
        char* body = NULL;
        int status = 200;
        if (name == NULL) {
            name = strdup("");
        }
        body = build_message(name);
        if (!body) {
            status = 400;
            body = strdup("Invalid name");
        }
        free(name);

        char header[256];
        int blen = (int)strlen(body);
        snprintf(header, sizeof(header),
                 "HTTP/1.1 %d %s\r\n"
                 "Content-Type: text/plain; charset=utf-8\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 status, (status == 200) ? "OK" : "Bad Request", blen);

        send(cfd, header, strlen(header), 0);
        send(cfd, body, blen, 0);
        free(body);
        close(cfd);
    }
    close(fd);
    return 0;
}