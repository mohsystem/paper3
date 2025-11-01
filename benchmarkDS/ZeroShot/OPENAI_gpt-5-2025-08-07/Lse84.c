#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT_C 5000
#define MAX_BODY_SIZE_C (1024 * 1024)

static char* str_trim(char *s) {
    if (!s) return s;
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = 0;
    return s;
}

static void tolower_str(char *s) {
    for (; s && *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static char* unquote_dup(const char *s) {
    size_t len = strlen(s);
    if (len >= 2 && ((s[0] == '"' && s[len-1] == '"') || (s[0] == '\'' && s[len-1] == '\''))) {
        char *out = (char*)malloc(len - 1);
        if (!out) return NULL;
        memcpy(out, s + 1, len - 2);
        out[len - 2] = '\0';
        return out;
    } else {
        char *out = strdup(s);
        return out;
    }
}

static char* strip_yaml_comments_dup(const char *s) {
    int inSingle = 0, inDouble = 0;
    size_t i;
    for (i = 0; s[i]; ++i) {
        char c = s[i];
        if (c == '\'' && !inDouble) inSingle = !inSingle;
        else if (c == '"' && !inSingle) inDouble = !inDouble;
        else if (c == '#' && !inSingle && !inDouble) {
            break;
        }
    }
    char *out = (char*)malloc(i + 1);
    if (!out) return NULL;
    memcpy(out, s, i);
    out[i] = '\0';
    return out;
}

static char* parse_yaml_type_dup(const char *yaml) {
    char *copy = strdup(yaml ? yaml : "");
    if (!copy) return NULL;
    char *saveptr = NULL;
    char *line = strtok_r(copy, "\r\n", &saveptr);
    char *result = NULL;
    while (line) {
        char *no_comment = strip_yaml_comments_dup(line);
        if (!no_comment) { line = strtok_r(NULL, "\r\n", &saveptr); continue; }
        char *trimmed = str_trim(no_comment);
        if (*trimmed && strncmp(trimmed, "---", 3) != 0) {
            char *colon = strchr(trimmed, ':');
            if (colon && colon != trimmed) {
                *colon = '\0';
                char *key = str_trim(trimmed);
                char *val = str_trim(colon + 1);
                char *key_lower = strdup(key);
                if (key_lower) {
                    tolower_str(key_lower);
                    if (strcmp(key_lower, "type") == 0) {
                        char *u = unquote_dup(val);
                        if (u) result = u;
                        free(key_lower);
                        free(no_comment);
                        break;
                    }
                    free(key_lower);
                }
            }
        }
        free(no_comment);
        line = strtok_r(NULL, "\r\n", &saveptr);
    }
    free(copy);
    return result; // may be NULL if not found
}

static char* processPayloadC(const char *payload) {
    if (!payload || strlen(payload) == 0) {
        return strdup("Error: missing 'payload' parameter.");
    }
    if (strlen(payload) > MAX_BODY_SIZE_C) {
        return strdup("Error: payload too large.");
    }
    char *type = NULL;
    type = parse_yaml_type_dup(payload);
    if (type) {
        char *tmp = strdup(type);
        if (!tmp) { free(type); return strdup("Error: internal server error."); }
        tolower_str(tmp);
        if (strcmp(tmp, "create") == 0) {
            free(type);
            free(tmp);
            return strdup("Error: 'Create' type is not allowed.");
        }
        free(type);
        free(tmp);
    }
    return strdup("OK");
}

static int send_all(int fd, const char *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t r = send(fd, buf + sent, len - sent, 0);
        if (r <= 0) return -1;
        sent += (size_t)r;
    }
    return 0;
}

static void send_response_c(int client_fd, int status, const char *body) {
    char header[512];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "Cache-Control: no-store\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     status, status == 200 ? "OK" : "Bad Request",
                     strlen(body));
    send_all(client_fd, header, (size_t)n);
    send_all(client_fd, body, strlen(body));
}

static char hex_to_byte(const char *h) {
    int hi = isdigit((unsigned char)h[0]) ? h[0]-'0' : (tolower((unsigned char)h[0]) - 'a' + 10);
    int lo = isdigit((unsigned char)h[1]) ? h[1]-'0' : (tolower((unsigned char)h[1]) - 'a' + 10);
    return (char)((hi << 4) | lo);
}

static char* url_decode_dup(const char *s) {
    size_t len = strlen(s);
    char *out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            out[oi++] = hex_to_byte(&s[i+1]);
            i += 2;
        } else if (s[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = '\0';
    return out;
}

static char* get_query_param_payload_dup(const char *query) {
    char *qcopy = strdup(query ? query : "");
    if (!qcopy) return NULL;
    char *saveptr = NULL;
    char *pair = strtok_r(qcopy, "&", &saveptr);
    char *result = NULL;
    while (pair) {
        char *eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            char *key = pair;
            char *val = eq + 1;
            char *key_dec = url_decode_dup(key);
            if (key_dec && strcmp(key_dec, "payload") == 0) {
                char *val_dec = url_decode_dup(val);
                if (val_dec) result = val_dec;
                free(key_dec);
                break;
            }
            free(key_dec);
        }
        pair = strtok_r(NULL, "&", &saveptr);
    }
    free(qcopy);
    return result;
}

static void run_server_c() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_C);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(server_fd); return; }
    if (listen(server_fd, 16) < 0) { perror("listen"); close(server_fd); return; }
    printf("C server listening on port %d\n", PORT_C);

    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }
        struct timeval tv; tv.tv_sec = 5; tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        char buffer[8192];
        ssize_t r = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (r <= 0) { close(client_fd); continue; }
        buffer[r] = '\0';

        char method[8] = {0}, path[4096] = {0}, version[16] = {0};
        sscanf(buffer, "%7s %4095s %15s", method, path, version);

        // Parse headers
        char *header_end = strstr(buffer, "\r\n\r\n");
        size_t header_len = header_end ? (size_t)(header_end - buffer) : (size_t)r;
        char *content_type = NULL;
        size_t content_length = 0;

        char *headers = strndup(buffer, header_len);
        if (headers) {
            char *saveptr = NULL;
            char *line = strtok_r(headers, "\r\n", &saveptr); // request line
            line = strtok_r(NULL, "\r\n", &saveptr);
            while (line) {
                char *colon = strchr(line, ':');
                if (colon) {
                    *colon = '\0';
                    char *name = str_trim(line);
                    char *value = str_trim(colon + 1);
                    for (char *p = name; *p; ++p) *p = (char)tolower((unsigned char)*p);
                    if (strcmp(name, "content-type") == 0) {
                        char *semi = strchr(value, ';');
                        if (semi) *semi = '\0';
                        content_type = strdup(str_trim(value));
                    } else if (strcmp(name, "content-length") == 0) {
                        content_length = (size_t)strtoul(value, NULL, 10);
                        if (content_length > MAX_BODY_SIZE_C) {
                            send_response_c(client_fd, 400, "Error: payload too large.");
                            free(headers);
                            if (content_type) free(content_type);
                            close(client_fd);
                            goto next_conn;
                        }
                    }
                }
                line = strtok_r(NULL, "\r\n", &saveptr);
            }
            free(headers);
        }

        char *payload = NULL;

        if (strcmp(method, "GET") == 0) {
            char *qm = strchr(path, '?');
            if (qm) {
                char *query = qm + 1;
                payload = get_query_param_payload_dup(query);
            }
        } else if (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0) {
            if (header_end) {
                char *body = header_end + 4;
                size_t body_len = (size_t)(buffer + r - body);
                if (content_type && strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
                    char *body_copy = (char*)malloc(body_len + 1);
                    if (body_copy) {
                        memcpy(body_copy, body, body_len);
                        body_copy[body_len] = '\0';
                        payload = get_query_param_payload_dup(body_copy);
                        free(body_copy);
                    }
                } else {
                    char *body_copy = (char*)malloc(body_len + 1);
                    if (body_copy) {
                        memcpy(body_copy, body, body_len);
                        body_copy[body_len] = '\0';
                        payload = body_copy; // treat entire body as payload
                    }
                }
            }
        }

        char *result = processPayloadC(payload ? payload : "");
        int status = (strncmp(result, "Error:", 6) == 0) ? 400 : 200;
        send_response_c(client_fd, status, result);
        free(result);
        if (payload) free(payload);
        if (content_type) free(content_type);
        close(client_fd);
    next_conn:
        ;
    }
    close(server_fd);
}

int main(void) {
    // 5 test cases
    char *r1 = processPayloadC(NULL);
    char *r2 = processPayloadC("type: Create");
    char *r3 = processPayloadC("type: Update");
    char *r4 = processPayloadC("name: test\nvalue: 123");
    char *r5 = processPayloadC("type: \"Create\"");
    printf("Test1 (no payload): %s\n", r1); free(r1);
    printf("Test2 (Create): %s\n", r2); free(r2);
    printf("Test3 (Update): %s\n", r3); free(r3);
    printf("Test4 (no type): %s\n", r4); free(r4);
    printf("Test5 (Type with quotes): %s\n", r5); free(r5);

    run_server_c();
    return 0;
}