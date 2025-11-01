/* Steps applied (high-level):
   1) Minimal web app with /api route reading ?payload=..., parse JSON to dict-like via simple parser, return "name".
   2) Security: input size limits, only string key/value pairs, robust URL decoding, error handling.
   3) No unsafe deserialization; POSIX sockets; GET only.
   4) Reviewed for bounds and errors.
   5) Final secure code.
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef _WIN32
// For simplicity, this sample assumes POSIX-like environment.
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#define MAX_PAYLOAD_LENGTH_C 2048
#define MAX_REQUEST_SIZE_C 4096

static size_t skip_ws_c(const char* s, size_t len, size_t i) {
    while (i < len && isspace((unsigned char)s[i])) i++;
    return i;
}

// Parses JSON string starting at s[start] which must be '"'
// Stores result into out (allocated by caller) up to outcap-1, returns next index after closing quote via *next_idx.
// Supports escapes \" \\ 
 \t \r
static int parse_json_string_c(const char* s, size_t len, size_t start, char* out, size_t outcap, size_t* next_idx) {
    if (start >= len || s[start] != '"') return 0;
    size_t i = start + 1;
    size_t k = 0;
    while (i < len) {
        char c = s[i];
        if (c == '\\') {
            if (i + 1 >= len) return 0;
            char n = s[i + 1];
            char v;
            if (n == '\\' || n == '"') v = n;
            else if (n == 'n') v = '\n';
            else if (n == 't') v = '\t';
            else if (n == 'r') v = '\r';
            else return 0;
            if (k + 1 >= outcap) return 0;
            out[k++] = v;
            i += 2;
        } else if (c == '"') {
            if (k >= outcap) return 0;
            out[k] = '\0';
            *next_idx = i + 1;
            return 1;
        } else {
            if (k + 1 >= outcap) return 0;
            out[k++] = c;
            i++;
        }
    }
    return 0;
}

typedef struct pair_s {
    char key[256];
    char val[1024];
} pair_t;

typedef struct dict_s {
    pair_t items[64];
    size_t count;
} dict_t;

// Parses a flat JSON object { "k":"v", ... } into dict
static int parse_simple_json_to_dict_c(const char* json, dict_t* out) {
    out->count = 0;
    size_t len = strlen(json);
    size_t a = 0, b = len;
    while (a < b && isspace((unsigned char)json[a])) a++;
    while (b > a && isspace((unsigned char)json[b - 1])) b--;
    if (b - a < 2) return 0;
    if (json[a] != '{' || json[b - 1] != '}') return 0;
    size_t i = a + 1;
    while (i < b - 1) {
        i = skip_ws_c(json, b, i);
        if (i >= b - 1) break;
        if (json[i] == ',') { i++; continue; }
        if (json[i] != '"') return 0;
        size_t next_i = 0;
        char key[256];
        if (!parse_json_string_c(json, b, i, key, sizeof(key), &next_i)) return 0;
        i = skip_ws_c(json, b, next_i);
        if (i >= b - 1 || json[i] != ':') return 0;
        i++;
        i = skip_ws_c(json, b, i);
        if (i >= b - 1 || json[i] != '"') return 0;
        char val[1024];
        if (!parse_json_string_c(json, b, i, val, sizeof(val), &next_i)) return 0;
        i = next_i;
        if (out->count >= 64) return 0;
        strncpy(out->items[out->count].key, key, sizeof(out->items[out->count].key) - 1);
        out->items[out->count].key[sizeof(out->items[out->count].key) - 1] = '\0';
        strncpy(out->items[out->count].val, val, sizeof(out->items[out->count].val) - 1);
        out->items[out->count].val[sizeof(out->items[out->count].val) - 1] = '\0';
        out->count++;
        i = skip_ws_c(json, b, i);
        if (i < b - 1) {
            if (json[i] == ',') i++;
            else if (json[i] == '}') break;
        }
    }
    return 1;
}

static const char* dict_get(const dict_t* d, const char* key) {
    for (size_t i = 0; i < d->count; i++) {
        if (strcmp(d->items[i].key, key) == 0) return d->items[i].val;
    }
    return NULL;
}

char* getNameFromPayload(const char* payload) {
    if (payload == NULL) return NULL;
    size_t n = strlen(payload);
    if (n == 0 || n > MAX_PAYLOAD_LENGTH_C) return NULL;
    dict_t d;
    if (!parse_simple_json_to_dict_c(payload, &d)) return NULL;
    const char* name = dict_get(&d, "name");
    if (name == NULL) return NULL;
    size_t L = strlen(name);
    char* out = (char*)malloc(L + 1);
    if (!out) return NULL;
    memcpy(out, name, L + 1);
    return out;
}

static char hex_val(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'a' && c <= 'f') return (char)(10 + c - 'a');
    if (c >= 'A' && c <= 'F') return (char)(10 + c - 'A');
    return -1;
}

static char* url_decode_c(const char* in) {
    size_t len = strlen(in);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = in[i];
        if (c == '+') {
            out[j++] = ' ';
        } else if (c == '%' && i + 2 < len) {
            char h1 = hex_val(in[i + 1]);
            char h2 = hex_val(in[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                out[j++] = (char)((h1 << 4) | h2);
                i += 2;
            } else {
                out[j++] = c;
            }
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static void send_http_response_c(int cfd, int code, const char* body) {
    char header[512];
    size_t body_len = strlen(body);
    const char* msg = "OK";
    if (code == 200) msg = "OK";
    else if (code == 400) msg = "Bad Request";
    else if (code == 404) msg = "Not Found";
    else if (code == 405) msg = "Method Not Allowed";
    else if (code == 413) msg = "Payload Too Large";
    else msg = "Internal Server Error";
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "Cache-Control: no-store\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     code, msg, body_len);
#ifdef _WIN32
    send(cfd, header, n, 0);
    send(cfd, body, (int)body_len, 0);
    closesocket(cfd);
#else
    send(cfd, header, n, 0);
    send(cfd, body, body_len, 0);
    close(cfd);
#endif
}

static void run_server_c() {
#ifdef _WIN32
    // Windows socket init omitted for brevity.
#endif
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) { fprintf(stderr, "socket failed\n"); return; }
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8084);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "bind failed\n");
#ifdef _WIN32
        closesocket(sfd);
#else
        close(sfd);
#endif
        return;
    }
    if (listen(sfd, 16) < 0) {
        fprintf(stderr, "listen failed\n");
#ifdef _WIN32
        closesocket(sfd);
#else
        close(sfd);
#endif
        return;
    }
    printf("C server listening on http://localhost:8084/api?payload={...}\n");
    for (;;) {
        struct sockaddr_in caddr;
#ifdef _WIN32
        int clen = sizeof(caddr);
#else
        socklen_t clen = sizeof(caddr);
#endif
        int cfd = accept(sfd, (struct sockaddr*)&caddr, &clen);
        if (cfd < 0) continue;
        char buf[MAX_REQUEST_SIZE_C + 1];
#ifdef _WIN32
        int n = recv(cfd, buf, MAX_REQUEST_SIZE_C, 0);
#else
        ssize_t n = recv(cfd, buf, MAX_REQUEST_SIZE_C, 0);
#endif
        if (n <= 0) {
            send_http_response_c(cfd, 400, "Bad Request");
            continue;
        }
        buf[n] = '\0';
        char* line_end = strstr(buf, "\r\n");
        if (!line_end) { send_http_response_c(cfd, 400, "Bad Request"); continue; }
        size_t line_len = (size_t)(line_end - buf);
        char first[1024];
        if (line_len >= sizeof(first)) { send_http_response_c(cfd, 400, "Bad Request"); continue; }
        memcpy(first, buf, line_len);
        first[line_len] = '\0';
        // Expect "GET /api?payload=... HTTP/1.1"
        if (strncmp(first, "GET ", 4) != 0) { send_http_response_c(cfd, 405, "Method Not Allowed"); continue; }
        char* sp1 = strchr(first, ' ');
        if (!sp1) { send_http_response_c(cfd, 400, "Bad Request"); continue; }
        char* sp2 = strchr(sp1 + 1, ' ');
        if (!sp2) { send_http_response_c(cfd, 400, "Bad Request"); continue; }
        size_t target_len = (size_t)(sp2 - (sp1 + 1));
        char target[1024];
        if (target_len >= sizeof(target)) { send_http_response_c(cfd, 400, "Bad Request"); continue; }
        memcpy(target, sp1 + 1, target_len);
        target[target_len] = '\0';
        // Split path and query
        char* qm = strchr(target, '?');
        char path[1024];
        char query[1024];
        if (qm) {
            size_t path_len = (size_t)(qm - target);
            if (path_len >= sizeof(path)) { send_http_response_c(cfd, 400, "Bad Request"); continue; }
            memcpy(path, target, path_len);
            path[path_len] = '\0';
            strncpy(query, qm + 1, sizeof(query) - 1);
            query[sizeof(query) - 1] = '\0';
        } else {
            strncpy(path, target, sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
            query[0] = '\0';
        }
        if (strcmp(path, "/api") != 0) { send_http_response_c(cfd, 404, "Not Found"); continue; }
        // Find payload parameter
        char* payload_val = NULL;
        char* qcopy = strdup(query);
        if (!qcopy) { send_http_response_c(cfd, 500, "Internal Server Error"); continue; }
        char* saveptr = NULL;
        for (char* tok = strtok_r(qcopy, "&", &saveptr); tok; tok = strtok_r(NULL, "&", &saveptr)) {
            char* eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                char* key = tok;
                char* val = eq + 1;
                if (strcmp(key, "payload") == 0) {
                    payload_val = strdup(val);
                    break;
                }
            } else {
                if (strcmp(tok, "payload") == 0) {
                    payload_val = strdup("");
                    break;
                }
            }
        }
        free(qcopy);
        if (!payload_val) { send_http_response_c(cfd, 400, "Missing 'payload' parameter"); continue; }
        char* decoded = url_decode_c(payload_val);
        free(payload_val);
        if (!decoded) { send_http_response_c(cfd, 500, "Internal Server Error"); continue; }
        if (strlen(decoded) > MAX_PAYLOAD_LENGTH_C) {
            free(decoded);
            send_http_response_c(cfd, 413, "Payload too large");
            continue;
        }
        char* name = getNameFromPayload(decoded);
        free(decoded);
        if (!name) {
            send_http_response_c(cfd, 400, "Invalid or missing 'name'");
            continue;
        }
        send_http_response_c(cfd, 200, name);
        free(name);
    }
}

int main(void) {
    // 5 test cases for core function
    const char* tests[] = {
        "{\"name\":\"Alice\"}",
        "{\"name\":\"Bob\",\"age\":\"30\"}",
        "{\"Name\":\"CaseSensitive\"}",
        "{invalid json}",
        "{\"name\":\"\"}"
    };
    for (int i = 0; i < 5; i++) {
        char* res = getNameFromPayload(tests[i]);
        if (res) {
            printf("Test %d OK: '%s'\n", i + 1, res);
            free(res);
        } else {
            printf("Test %d Error\n", i + 1);
        }
    }
    run_server_c();
    return 0;
}