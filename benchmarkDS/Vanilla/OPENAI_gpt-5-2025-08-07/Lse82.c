// C single-file minimal HTTP server for /api that extracts "name" from JSON payload query param.
// Note: POSIX sockets used for simplicity.
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

char* str_trim(char* s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

char* url_decode_c(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (c == '+') {
            out[j++] = ' ';
        } else if (c == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            out[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

typedef struct {
    char** keys;
    char** values;
    size_t size;
} strmap;

void strmap_init(strmap* m) {
    m->keys = NULL;
    m->values = NULL;
    m->size = 0;
}
void strmap_put(strmap* m, const char* k, const char* v) {
    m->keys = (char**)realloc(m->keys, sizeof(char*) * (m->size + 1));
    m->values = (char**)realloc(m->values, sizeof(char*) * (m->size + 1));
    m->keys[m->size] = strdup(k);
    m->values[m->size] = strdup(v);
    m->size++;
}
const char* strmap_get(strmap* m, const char* k) {
    for (size_t i = 0; i < m->size; i++) {
        if (strcmp(m->keys[i], k) == 0) return m->values[i];
    }
    return NULL;
}
void strmap_free(strmap* m) {
    for (size_t i = 0; i < m->size; i++) {
        free(m->keys[i]);
        free(m->values[i]);
    }
    free(m->keys);
    free(m->values);
    m->size = 0;
}

strmap parse_query_c(const char* qraw) {
    strmap m; strmap_init(&m);
    if (!qraw) return m;
    char* q = strdup(qraw);
    char* saveptr1 = NULL;
    char* token = strtok_r(q, "&", &saveptr1);
    while (token) {
        char* kv = strdup(token);
        char* eq = strchr(kv, '=');
        if (eq) {
            *eq = '\0';
            char* key = url_decode_c(kv);
            char* val = url_decode_c(eq + 1);
            strmap_put(&m, key, val);
            free(key);
            free(val);
        } else {
            char* key = url_decode_c(kv);
            strmap_put(&m, key, "");
            free(key);
        }
        free(kv);
        token = strtok_r(NULL, "&", &saveptr1);
    }
    free(q);
    return m;
}

char* unescape_json_string_c(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    size_t j = 0;
    int esc = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (esc) {
            switch (c) {
                case '"': out[j++] = '"'; break;
                case '\\': out[j++] = '\\'; break;
                case '/': out[j++] = '/'; break;
                case 'b': out[j++] = '\b'; break;
                case 'f': out[j++] = '\f'; break;
                case 'n': out[j++] = '\n'; break;
                case 'r': out[j++] = '\r'; break;
                case 't': out[j++] = '\t'; break;
                case 'u':
                    if (i + 4 < len) {
                        char hex[5] = { s[i+1], s[i+2], s[i+3], s[i+4], 0 };
                        int cp = (int)strtol(hex, NULL, 16);
                        out[j++] = (char)cp;
                        i += 4;
                    } else {
                        out[j++] = '\\'; out[j++] = 'u';
                    }
                    break;
                default: out[j++] = c;
            }
            esc = 0;
        } else if (c == '\\') {
            esc = 1;
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

// Minimal flat JSON object parser into strmap
strmap parse_flat_json_object_c(const char* json) {
    strmap m; strmap_init(&m);
    if (!json) return m;
    char* s0 = strdup(json);
    char* s = str_trim(s0);
    size_t L = strlen(s);
    if (L > 0 && s[0] == '{') { s++; L--; }
    if (L > 0 && s[L-1] == '}') { s[L-1] = '\0'; }

    char* buf = strdup(s);
    size_t len = strlen(buf);
    size_t start = 0;
    int inQuotes = 0, esc = 0;
    for (size_t i = 0; i <= len; i++) {
        char c = (i < len) ? buf[i] : ',';
        if (inQuotes) {
            if (esc) esc = 0;
            else if (c == '\\') esc = 1;
            else if (c == '"') inQuotes = 0;
        } else {
            if (c == '"') inQuotes = 1;
            else if (c == ',' || i == len) {
                size_t partLen = i - start;
                if (partLen > 0) {
                    char* part = (char*)malloc(partLen + 1);
                    memcpy(part, buf + start, partLen);
                    part[partLen] = 0;
                    char* t = str_trim(part);
                    // find colon not in quotes
                    int inQ = 0, es = 0;
                    int idx = -1;
                    for (int k = 0; t[k]; k++) {
                        char cc = t[k];
                        if (inQ) {
                            if (es) es = 0;
                            else if (cc == '\\') es = 1;
                            else if (cc == '"') inQ = 0;
                        } else {
                            if (cc == '"') inQ = 1;
                            else if (cc == ':') { idx = k; break; }
                        }
                    }
                    if (idx >= 0) {
                        t[idx] = 0;
                        char* key = str_trim(t);
                        char* val = str_trim(t + idx + 1);
                        if (key[0] == '"' && key[strlen(key)-1] == '"' && strlen(key) >= 2) {
                            key[strlen(key)-1] = 0;
                            key++;
                        }
                        char* finalVal = NULL;
                        if (val[0] == '"' && val[strlen(val)-1] == '"' && strlen(val) >= 2) {
                            val[strlen(val)-1] = 0;
                            val++;
                            char* un = unescape_json_string_c(val);
                            finalVal = un;
                        } else {
                            finalVal = strdup(val);
                        }
                        strmap_put(&m, key, finalVal);
                        free(finalVal);
                    }
                    free(part);
                }
                start = i + 1;
            }
        }
    }
    free(buf);
    free(s0);
    return m;
}

char* get_name_from_payload_c(const char* payload) {
    strmap m = parse_flat_json_object_c(payload);
    const char* v = strmap_get(&m, "name");
    char* out = strdup(v ? v : "");
    strmap_free(&m);
    return out;
}

void send_response(int client, int status, const char* body) {
    char header[256];
    int blen = (int)strlen(body);
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
             status, blen);
    send(client, header, strlen(header), 0);
    send(client, body, blen, 0);
}

void serve_c() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(server_fd); return; }
    if (listen(server_fd, 16) < 0) { close(server_fd); return; }

    // printf("C server running on http://localhost:8080/api?payload=%7B%22name%22%3A%22World%22%7D\n");
    while (1) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) continue;
        char buf[4096];
        ssize_t n = recv(client, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(client); continue; }
        buf[n] = 0;
        char method[8], path[2048], ver[16];
        if (sscanf(buf, "%7s %2047s %15s", method, path, ver) != 3) {
            send_response(client, 400, "Bad Request");
            close(client);
            continue;
        }
        if (strcmp(method, "GET") != 0) {
            send_response(client, 405, "Method Not Allowed");
            close(client);
            continue;
        }
        char* qmark = strchr(path, '?');
        char target[2048]; target[0] = 0;
        char* query = NULL;
        if (qmark) {
            size_t tlen = (size_t)(qmark - path);
            memcpy(target, path, tlen);
            target[tlen] = 0;
            query = qmark + 1;
        } else {
            strncpy(target, path, sizeof(target)-1);
            target[sizeof(target)-1] = 0;
        }
        if (strcmp(target, "/api") != 0) {
            send_response(client, 404, "Not Found");
            close(client);
            continue;
        }
        strmap params = parse_query_c(query ? query : "");
        const char* payload = strmap_get(&params, "payload");
        if (!payload) {
            strmap_free(&params);
            send_response(client, 400, "Missing payload parameter");
            close(client);
            continue;
        }
        char* name = get_name_from_payload_c(payload);
        send_response(client, 200, name);
        free(name);
        strmap_free(&params);
        close(client);
    }
}

int main() {
    // 5 test cases
    const char* tests[] = {
        "{\"name\":\"Alice\"}",
        "{\"name\":\"Bob\",\"age\":30}",
        "{\"age\":22,\"name\":\"Carol\"}",
        "{\"name\":\"D\\\"E\"}",
        "{\"name\":\"Eve Adams\",\"city\":\"NY\"}"
    };
    for (int i = 0; i < 5; i++) {
        char* name = get_name_from_payload_c(tests[i]);
        printf("%s\n", name);
        free(name);
    }
    // Start server
    serve_c();
    return 0;
}