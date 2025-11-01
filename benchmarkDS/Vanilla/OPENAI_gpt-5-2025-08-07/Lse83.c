#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(_WIN32)
// Windows sockets not included; this example uses POSIX sockets for the server.
// You can compile/run on Unix-like systems or WSL.
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

// Helper to URL-decode in place
static void urldecode(char* dst, const char* src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = (char)(16 * a + b);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Extract 'name' from JSON or form-urlencoded payload
char* extract_name(const char* payload) {
    if (!payload) return NULL;
    // Try simple JSON scan: look for "name" : "value"
    const char* p = strstr(payload, "\"name\"");
    if (p) {
        p = strchr(p, ':');
        if (p) {
            p++;
            while (isspace((unsigned char)*p)) p++;
            if (*p == '\"') {
                p++;
                const char* start = p;
                int escaped = 0;
                while (*p) {
                    if (!escaped && *p == '\"') break;
                    if (*p == '\\' && !escaped) escaped = 1;
                    else escaped = 0;
                    p++;
                }
                if (*p == '\"') {
                    size_t len = (size_t)(p - start);
                    char* val = (char*)malloc(len + 1);
                    if (!val) return NULL;
                    // Unescape simple sequences
                    size_t i = 0, j = 0;
                    while (i < len) {
                        if (start[i] == '\\' && i + 1 < len) {
                            char c = start[i + 1];
                            if (c == 'n') val[j++] = '\n';
                            else if (c == 'r') val[j++] = '\r';
                            else if (c == 't') val[j++] = '\t';
                            else val[j++] = c;
                            i += 2;
                        } else {
                            val[j++] = start[i++];
                        }
                    }
                    val[j] = '\0';
                    return val;
                }
            }
        }
    }
    // Try form-urlencoded: name=...
    const char* q = strstr(payload, "name=");
    if (q) {
        q += 5;
        const char* end = strchr(q, '&');
        size_t len = end ? (size_t)(end - q) : strlen(q);
        char* raw = (char*)malloc(len + 1);
        if (!raw) return NULL;
        memcpy(raw, q, len);
        raw[len] = '\0';
        char* decoded = (char*)malloc(len + 1);
        if (!decoded) { free(raw); return NULL; }
        urldecode(decoded, raw);
        free(raw);
        return decoded;
    }
    return NULL;
}

#ifndef _WIN32
void run_server_c() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "C server socket creation failed\n");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8083);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "C bind failed\n");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 5) < 0) {
        fprintf(stderr, "C listen failed\n");
        close(server_fd);
        return;
    }
    printf("C server running at http://localhost:8083/\n");
    for (;;) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) break;
        char buf[4096];
        ssize_t n;
        size_t total = 0;
        char *req = NULL;
        size_t req_cap = 0;
        // Read headers
        for (;;) {
            n = recv(client, buf, sizeof(buf), 0);
            if (n <= 0) break;
            if (total + (size_t)n + 1 > req_cap) {
                size_t newcap = (req_cap == 0 ? 8192 : req_cap * 2);
                while (newcap < total + (size_t)n + 1) newcap *= 2;
                char* tmp = (char*)realloc(req, newcap);
                if (!tmp) { free(req); req = NULL; break; }
                req = tmp;
                req_cap = newcap;
            }
            memcpy(req + total, buf, (size_t)n);
            total += (size_t)n;
            req[total] = '\0';
            if (strstr(req, "\r\n\r\n")) break;
        }
        if (!req) { close(client); continue; }
        // Parse Content-Length
        char* header_end = strstr(req, "\r\n\r\n");
        char* body = header_end ? header_end + 4 : NULL;
        int content_length = 0;
        char* cl = strcasestr(req, "Content-Length:");
        if (cl) {
            content_length = atoi(cl + strlen("Content-Length:"));
        }
        size_t body_have = body ? (total - (size_t)(body - req)) : 0;
        // Read remaining body
        while (body && body_have < (size_t)content_length) {
            n = recv(client, buf, sizeof(buf), 0);
            if (n <= 0) break;
            if (total + (size_t)n + 1 > req_cap) {
                size_t newcap = req_cap * 2;
                while (newcap < total + (size_t)n + 1) newcap *= 2;
                char* tmp = (char*)realloc(req, newcap);
                if (!tmp) { free(req); req = NULL; break; }
                req = tmp;
                req_cap = newcap;
            }
            memcpy(req + total, buf, (size_t)n);
            total += (size_t)n;
            req[total] = '\0';
            // update body pointer after realloc
            header_end = strstr(req, "\r\n\r\n");
            body = header_end ? header_end + 4 : NULL;
            body_have = body ? (total - (size_t)(body - req)) : 0;
        }
        char* payload = NULL;
        if (content_length > 0 && body && (size_t)content_length <= body_have) {
            payload = (char*)malloc((size_t)content_length + 1);
            memcpy(payload, body, (size_t)content_length);
            payload[content_length] = '\0';
        } else {
            // GET query as payload
            char* first_line_end = strstr(req, "\r\n");
            if (first_line_end) {
                size_t line_len = (size_t)(first_line_end - req);
                char* line = (char*)malloc(line_len + 1);
                memcpy(line, req, line_len);
                line[line_len] = '\0';
                // Method SP Path SP HTTP
                char method[16], path[2048], httpver[32];
                method[0]=path[0]=httpver[0]='\0';
                sscanf(line, "%15s %2047s %31s", method, path, httpver);
                free(line);
                char* q = strchr(path, '?');
                if (q) {
                    q++;
                    payload = strdup(q);
                }
            }
        }

        char* name = extract_name(payload ? payload : "");
        if (name) {
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\n\r\n",
                     strlen(name));
            send(client, header, strlen(header), 0);
            send(client, name, strlen(name), 0);
        } else {
            const char* msg = "Field 'name' not found";
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\n\r\n",
                     strlen(msg));
            send(client, header, strlen(header), 0);
            send(client, msg, strlen(msg), 0);
        }

        if (payload) free(payload);
        if (name) free(name);
        free(req);
        close(client);
    }
    close(server_fd);
}
#endif

int main() {
#if !defined(_WIN32)
    if (fork() == 0) {
        run_server_c();
        return 0;
    }
    printf("C server started in background at http://localhost:8083/\n");
#else
    printf("Note: C example uses POSIX sockets; run on Unix-like systems or WSL.\n");
#endif

    // 5 test cases
    const char* tests[] = {
        "{\"name\":\"Alice\"}",
        "{\"age\":30}",
        "{\"name\":\"Bob\",\"x\":1}",
        "name=Carol&x=1",
        "{\"greet\":\"hi\",\"name\":\"A\\\"B C\"}"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = extract_name(tests[i]);
        printf("Test %d: %s\n", i + 1, res ? res : "null");
        if (res) free(res);
    }
    // Keep main alive a bit (optional)
#if !defined(_WIN32)
    pause();
#endif
    return 0;
}