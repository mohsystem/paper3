#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_REQ_SIZE 8192
#define MAX_QUERY_LEN 4096
#define MAX_JSON_LEN 2048
#define MAX_NAME_LEN 256

static void send_response(int client_fd, int code, const char *body) {
    const char *status;
    switch (code) {
        case 200: status = "200 OK"; break;
        case 400: status = "400 Bad Request"; break;
        case 404: status = "404 Not Found"; break;
        case 405: status = "405 Method Not Allowed"; break;
        default:  status = "500 Internal Server Error"; break;
    }
    char header[256];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %s\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     status, body ? strlen(body) : 0);
    if (n < 0) return;
    send(client_fd, header, (size_t)n, 0);
    if (body && strlen(body) > 0) {
        send(client_fd, body, strlen(body), 0);
    }
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int url_decode(const char *in, char *out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        if (oi + 1 >= outsz) return -1;
        if (in[i] == '+') {
            out[oi++] = ' ';
        } else if (in[i] == '%' && in[i+1] && in[i+2]) {
            int h1 = hexval(in[i+1]);
            int h2 = hexval(in[i+2]);
            if (h1 < 0 || h2 < 0) return -1;
            out[oi++] = (char)((h1 << 4) | h2);
            i += 2;
        } else {
            out[oi++] = in[i];
        }
    }
    out[oi] = '\0';
    return 0;
}

// Minimal JSON parser for flat {"key":"value",...}. Returns dynamically allocated copy of "name" or NULL on error.
// The caller must free the returned pointer.
static char* parse_simple_json_and_get_name(const char *json) {
    if (!json) return NULL;
    size_t len = strlen(json);
    if (len == 0 || len > MAX_JSON_LEN) return NULL;

    size_t i = 0;
    // skip ws
    while (i < len && (json[i]==' '||json[i]=='\n'||json[i]=='\r'||json[i]=='\t')) i++;
    if (i >= len || json[i] != '{') return NULL;
    i++;

    char *name_value = NULL;

    while (1) {
        // skip ws
        while (i < len && (json[i]==' '||json[i]=='\n'||json[i]=='\r'||json[i]=='\t')) i++;
        if (i < len && json[i] == '}') { i++; break; }
        if (i >= len || json[i] != '"') { free(name_value); return NULL; }
        i++;

        // parse key
        char keybuf[128];
        size_t kpos = 0;
        while (i < len) {
            char c = json[i++];
            if (c == '"') break;
            if (c == '\\') {
                if (i >= len) { free(name_value); return NULL; }
                char e = json[i++];
                if (e == '"' || e == '\\' || e == '/') c = e;
                else if (e == 'b') c = '\b';
                else if (e == 'f') c = '\f';
                else if (e == 'n') c = '\n';
                else if (e == 'r') c = '\r';
                else if (e == 't') c = '\t';
                else { free(name_value); return NULL; }
            }
            if (kpos + 1 >= sizeof(keybuf)) { free(name_value); return NULL; }
            keybuf[kpos++] = c;
        }
        keybuf[kpos] = '\0';

        // skip ws
        while (i < len && (json[i]==' '||json[i]=='\n'||json[i]=='\r'||json[i]=='\t')) i++;
        if (i >= len || json[i] != ':') { free(name_value); return NULL; }
        i++;
        while (i < len && (json[i]==' '||json[i]=='\n'||json[i]=='\r'||json[i]=='\t')) i++;

        if (i >= len || json[i] != '"') { free(name_value); return NULL; }
        i++;

        // parse value string
        char valbuf[MAX_JSON_LEN + 1];
        size_t vpos = 0;
        while (i < len) {
            char c = json[i++];
            if (c == '"') break;
            if (c == '\\') {
                if (i >= len) { free(name_value); return NULL; }
                char e = json[i++];
                if (e == '"' || e == '\\' || e == '/') c = e;
                else if (e == 'b') c = '\b';
                else if (e == 'f') c = '\f';
                else if (e == 'n') c = '\n';
                else if (e == 'r') c = '\r';
                else if (e == 't') c = '\t';
                else { free(name_value); return NULL; }
            }
            if (vpos + 1 >= sizeof(valbuf)) { free(name_value); return NULL; }
            valbuf[vpos++] = c;
        }
        valbuf[vpos] = '\0';

        if (strcmp(keybuf, "name") == 0) {
            // replace previous if any
            free(name_value);
            size_t nlen = strlen(valbuf);
            if (nlen == 0 || nlen > MAX_NAME_LEN) { free(name_value); return NULL; }
            name_value = (char*)malloc(nlen + 1);
            if (!name_value) return NULL;
            memcpy(name_value, valbuf, nlen + 1);
        }

        // skip ws
        while (i < len && (json[i]==' '||json[i]=='\n'||json[i]=='\r'||json[i]=='\t')) i++;
        if (i < len && json[i] == ',') {
            i++;
            continue;
        } else if (i < len && json[i] == '}') {
            i++;
            break;
        } else if (i >= len) {
            break;
        } else {
            free(name_value);
            return NULL;
        }
    }

    // skip trailing ws
    while (i < len && (json[i]==' '||json[i]=='\n'||json[i]=='\r'||json[i]=='\t')) i++;
    if (i != len) { free(name_value); return NULL; }

    return name_value;
}

static void handle_client(int client_fd) {
    char reqbuf[MAX_REQ_SIZE + 1];
    ssize_t n = recv(client_fd, reqbuf, MAX_REQ_SIZE, 0);
    if (n <= 0) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    reqbuf[n] = '\0';

    // parse first line
    char *line_end = strstr(reqbuf, "\r\n");
    if (!line_end) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    *line_end = '\0';
    char method[8], target[4096], version[16];
    method[0]=target[0]=version[0]='\0';
    if (sscanf(reqbuf, "%7s %4095s %15s", method, target, version) != 3) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    if (strcmp(method, "GET") != 0) {
        send_response(client_fd, 405, "Method Not Allowed");
        close(client_fd);
        return;
    }

    // extract path and query
    char *qmark = strchr(target, '?');
    char path[2048]; path[0]='\0';
    char query[MAX_QUERY_LEN + 1]; query[0]='\0';
    if (qmark) {
        size_t plen = (size_t)(qmark - target);
        if (plen >= sizeof(path)) { send_response(client_fd, 400, "Bad Request"); close(client_fd); return; }
        memcpy(path, target, plen);
        path[plen] = '\0';
        size_t qlen = strlen(qmark + 1);
        if (qlen > MAX_QUERY_LEN) { send_response(client_fd, 400, "Bad Request"); close(client_fd); return; }
        memcpy(query, qmark + 1, qlen + 1);
    } else {
        if (strlen(target) >= sizeof(path)) { send_response(client_fd, 400, "Bad Request"); close(client_fd); return; }
        strcpy(path, target);
    }

    if (strcmp(path, "/api") != 0) {
        send_response(client_fd, 404, "Not Found");
        close(client_fd);
        return;
    }

    if (query[0] == '\0') {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }

    // find payload param
    char *payload_raw = NULL;
    char *saveptr = NULL;
    char qcopy[MAX_QUERY_LEN + 1];
    strncpy(qcopy, query, sizeof(qcopy) - 1);
    qcopy[sizeof(qcopy) - 1] = '\0';
    char *tok = strtok_r(qcopy, "&", &saveptr);
    while (tok) {
        char *eq = strchr(tok, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(tok, "payload") == 0) {
                payload_raw = eq + 1;
                break;
            }
        }
        tok = strtok_r(NULL, "&", &saveptr);
    }

    if (!payload_raw) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }

    if (strlen(payload_raw) > MAX_JSON_LEN * 3) { // encoded may be longer
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }

    char json[MAX_JSON_LEN + 1];
    if (url_decode(payload_raw, json, sizeof(json)) != 0) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }

    char *name = parse_simple_json_and_get_name(json);
    if (!name) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }

    send_response(client_fd, 200, name);
    free(name);
    close(client_fd);
}

static void run_server(uint16_t port, int max_requests) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return;
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(s);
        return;
    }
    if (listen(s, 8) < 0) {
        close(s);
        return;
    }
    for (int i = 0; i < max_requests; ++i) {
        int c = accept(s, NULL, NULL);
        if (c < 0) break;
        handle_client(c);
    }
    close(s);
}

static char* http_get(const char *host, uint16_t port, const char *path) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return strdup("0:");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(s);
        return strdup("0:");
    }
    char req[1024];
    snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    send(s, req, strlen(req), 0);

    char buf[2048];
    size_t cap = 4096;
    size_t total = 0;
    char *resp = (char*)malloc(cap);
    if (!resp) { close(s); return strdup("0:"); }
    ssize_t n;
    while ((n = recv(s, buf, sizeof(buf), 0)) > 0) {
        if (total + (size_t)n >= cap) {
            size_t newcap = cap * 2;
            char *tmp = (char*)realloc(resp, newcap);
            if (!tmp) { free(resp); close(s); return strdup("0:"); }
            resp = tmp;
            cap = newcap;
        }
        memcpy(resp + total, buf, (size_t)n);
        total += (size_t)n;
    }
    close(s);
    resp[total] = '\0';

    // parse status code
    char *sp = strchr(resp, ' ');
    int code = 0;
    if (sp && strlen(sp) >= 4) code = atoi(sp + 1);
    char *hdr_end = strstr(resp, "\r\n\r\n");
    char *body = hdr_end ? hdr_end + 4 : resp + total;
    size_t body_len = hdr_end ? (resp + total - body) : 0;

    // build "code:body"
    char codebuf[16];
    snprintf(codebuf, sizeof(codebuf), "%d:", code);
    size_t outlen = strlen(codebuf) + body_len;
    char *out = (char*)malloc(outlen + 1);
    if (!out) { free(resp); return strdup("0:"); }
    strcpy(out, codebuf);
    if (body_len > 0) memcpy(out + strlen(codebuf), body, body_len);
    out[outlen] = '\0';
    free(resp);
    return out;
}

int main(void) {
    uint16_t port = 8090;
    pid_t pid = fork();
    if (pid < 0) return 1;
    if (pid == 0) {
        run_server(port, 5);
        return 0;
    } else {
        // Give server a moment
        usleep(300000);

        char *res1 = http_get("127.0.0.1", port, "/api?payload=%7B%22name%22%3A%22Alice%22%7D");
        printf("%s\n", res1); free(res1);

        char *res2 = http_get("127.0.0.1", port, "/api?payload=%7B%22name%22%3A%22Bob%22%2C%22age%22%3A%2230%22%7D");
        printf("%s\n", res2); free(res2);

        char *res3 = http_get("127.0.0.1", port, "/api");
        printf("%s\n", res3); free(res3);

        char *res4 = http_get("127.0.0.1", port, "/api?payload=%7B%22name%22%3ABob%7D");
        printf("%s\n", res4); free(res4);

        char *res5 = http_get("127.0.0.1", port, "/api?payload=%7B%22title%22%3A%22Engineer%22%7D");
        printf("%s\n", res5); free(res5);

        return 0;
    }
}