#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(_WIN32)
// For simplicity, the networking web server portion is not implemented on Windows in this single-file example.
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#endif

static size_t skip_ws_forward(const char* s, size_t i) {
    while (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t') i++;
    return i;
}

typedef struct {
    int ok;
    char* value;
    size_t nextIndex;
} ParseResult;

static int is_hex(char c) {
    return (c >= '0' && c <= '9') || ( (c|32) >= 'a' && (c|32) <= 'f');
}

static ParseResult parse_json_string_c(const char* s, size_t n, size_t startQuoteIdx) {
    ParseResult pr; pr.ok = 0; pr.value = NULL; pr.nextIndex = 0;
    if (startQuoteIdx >= n || s[startQuoteIdx] != '\"') return pr;
    // Use dynamic buffer
    size_t cap = 64, len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) return pr;
    size_t i = startQuoteIdx + 1;
    while (i < n) {
        char c = s[i];
        if (c == '\"') {
            if (len + 1 >= cap) { char* nb = realloc(buf, cap*2); if (!nb) { free(buf); return pr; } buf = nb; cap*=2; }
            buf[len] = '\0';
            pr.ok = 1;
            pr.value = buf;
            pr.nextIndex = i + 1;
            return pr;
        } else if (c == '\\') {
            if (i + 1 >= n) { free(buf); return pr; }
            char esc = s[++i];
            char outc = 0;
            if (esc == '\"') outc = '\"';
            else if (esc == '\\') outc = '\\';
            else if (esc == '/') outc = '/';
            else if (esc == 'b') outc = '\b';
            else if (esc == 'f') outc = '\f';
            else if (esc == 'n') outc = '\n';
            else if (esc == 'r') outc = '\r';
            else if (esc == 't') outc = '\t';
            else if (esc == 'u') {
                if (i + 4 >= n) { free(buf); return pr; }
                if (!(is_hex(s[i+1]) && is_hex(s[i+2]) && is_hex(s[i+3]) && is_hex(s[i+4]))) { free(buf); return pr; }
                char hex[5]; memcpy(hex, s + i + 1, 4); hex[4] = '\0';
                int code = (int)strtol(hex, NULL, 16);
                outc = (char)code;
                i += 4;
            } else {
                free(buf); return pr;
            }
            if (len + 1 >= cap) { char* nb = realloc(buf, cap*2); if (!nb) { free(buf); return pr; } buf = nb; cap*=2; }
            buf[len++] = outc;
            i++;
        } else {
            if ((unsigned char)c <= 0x1F) { free(buf); return pr; }
            if (len + 1 >= cap) { char* nb = realloc(buf, cap*2); if (!nb) { free(buf); return pr; } buf = nb; cap*=2; }
            buf[len++] = c;
            i++;
        }
    }
    free(buf);
    return pr;
}

static long index_of_json_string_key_c(const char* s, size_t n, const char* key) {
    size_t i = 0;
    size_t keylen = strlen(key);
    while (i < n) {
        char* q = strchr(s + i, '\"');
        if (!q) return -1;
        size_t quote = (size_t)(q - s);
        ParseResult pr = parse_json_string_c(s, n, quote);
        if (!pr.ok) return -1;
        int match = (strlen(pr.value) == keylen) && (memcmp(pr.value, key, keylen) == 0);
        size_t next = pr.nextIndex;
        free(pr.value);
        if (match) {
            size_t after = skip_ws_forward(s, next);
            if (after < n && s[after] == ':') return (long)after;
        }
        i = next;
    }
    return -1;
}

// Returns 1 on success and writes name to out buffer; returns 0 on failure.
int extract_name_from_payload(const char* payload, char* out, size_t outsz) {
    if (!payload || !out || outsz == 0) return 0;
    size_t n = strlen(payload);
    if (n == 0 || n > 4096) return 0;
    // Trim
    const char* start = payload;
    while (*start && (*start==' '||*start=='\n'||*start=='\r'||*start=='\t')) start++;
    const char* end = payload + n - 1;
    while (end >= start && (*end==' '||*end=='\n'||*end=='\r'||*end=='\t')) end--;
    if (start > end) return 0;
    if (*start != '{' || *end != '}') return 0;
    size_t len = (size_t)(end - start + 1);
    long keyPos = index_of_json_string_key_c(start, len, "name");
    if (keyPos < 0) return 0;
    size_t colon = (size_t)keyPos + 1;
    colon = skip_ws_forward(start, colon);
    if (colon >= len || start[colon] != '\"') return 0;
    ParseResult pr = parse_json_string_c(start, len, colon);
    if (!pr.ok) return 0;
    // Copy safely
    size_t copyLen = strlen(pr.value);
    if (copyLen + 1 > outsz) { free(pr.value); return 0; }
    memcpy(out, pr.value, copyLen + 1);
    free(pr.value);
    return 1;
}

#if !defined(_WIN32)
static void send_http_response(int fd, int code, const char* body) {
    char header[512];
    size_t blen = body ? strlen(body) : 0;
    const char* msg = "Error";
    if (code == 200) msg = "OK";
    else if (code == 400) msg = "Bad Request";
    else if (code == 404) msg = "Not Found";
    else if (code == 405) msg = "Method Not Allowed";
    else if (code == 413) msg = "Payload Too Large";
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     code, msg, blen);
    send(fd, header, (size_t)n, 0);
    if (blen) send(fd, body, blen, 0);
}

static void handle_client(int cfd) {
    char buf[4096];
    ssize_t n = recv(cfd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(cfd); return; }
    buf[n] = 0;
    // Parse first line
    char method[8]={0}, path[2048]={0}, version[16]={0};
    if (sscanf(buf, "%7s %2047s %15s", method, path, version) != 3) {
        send_http_response(cfd, 400, "Bad Request");
        close(cfd);
        return;
    }
    if (strcmp(method, "GET") != 0) {
        send_http_response(cfd, 405, "Method Not Allowed");
        close(cfd);
        return;
    }
    // Split path and query
    char route[2048]={0}, query[2048]={0};
    char* qmark = strchr(path, '?');
    if (qmark) {
        size_t rlen = (size_t)(qmark - path);
        if (rlen >= sizeof(route)) rlen = sizeof(route)-1;
        memcpy(route, path, rlen); route[rlen] = 0;
        strncpy(query, qmark + 1, sizeof(query)-1);
    } else {
        strncpy(route, path, sizeof(route)-1);
        query[0] = 0;
    }
    if (strcmp(route, "/api") != 0) {
        send_http_response(cfd, 404, "Not Found");
        close(cfd);
        return;
    }
    // Find payload= in query
    char rawPayload[16385]={0};
    int found = 0;
    char* saveptr = NULL;
    char* token = strtok_r(query, "&", &saveptr);
    while (token) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = 0;
            const char* k = token;
            const char* v = eq + 1;
            if (strcmp(k, "payload") == 0) {
                strncpy(rawPayload, v, sizeof(rawPayload)-1);
                found = 1;
                break;
            }
        }
        token = strtok_r(NULL, "&", &saveptr);
    }
    if (!found) {
        send_http_response(cfd, 400, "Missing payload parameter");
        close(cfd);
        return;
    }
    if (strlen(rawPayload) > 16384) {
        send_http_response(cfd, 413, "Payload too large");
        close(cfd);
        return;
    }
    // URL decode rawPayload
    char decoded[8192]={0};
    size_t di = 0;
    for (size_t i = 0; rawPayload[i] && di + 1 < sizeof(decoded); ++i) {
        if (rawPayload[i] == '+') {
            decoded[di++] = ' ';
        } else if (rawPayload[i] == '%' && isxdigit((unsigned char)rawPayload[i+1]) && isxdigit((unsigned char)rawPayload[i+2])) {
            char hex[3] = { rawPayload[i+1], rawPayload[i+2], 0 };
            int v = (int)strtol(hex, NULL, 16);
            decoded[di++] = (char)v;
            i += 2;
        } else {
            decoded[di++] = rawPayload[i];
        }
    }
    decoded[di] = 0;

    char name[512]={0};
    if (!extract_name_from_payload(decoded, name, sizeof(name)) || name[0] == 0) {
        send_http_response(cfd, 400, "Invalid payload or missing name");
        close(cfd);
        return;
    }
    send_http_response(cfd, 200, name);
    close(cfd);
}

static void start_server() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(0x7F000001); // 127.0.0.1
    addr.sin_port = htons(8083);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(sfd); return; }
    if (listen(sfd, 8) != 0) { close(sfd); return; }
    for (int i = 0; i < 5; ++i) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) break;
        handle_client(cfd);
    }
    close(sfd);
}
#endif

int main(void) {
    // 5 test cases
    const char* tests[5] = {
        "{\"name\":\"Alice\"}",
        "{\"age\":30}",
        "not json",
        "{\"name\":\"A\\u004c\\u0069\\u0063\\u0065\"}",
        "{\"name\":\"Bob \\\"The Builder\\\"\"}"
    };
    for (int i = 0; i < 5; ++i) {
        char out[256] = {0};
        int ok = extract_name_from_payload(tests[i], out, sizeof(out));
        if (!ok || out[0] == 0) printf("(null)\n"); else printf("%s\n", out);
    }
#if !defined(_WIN32)
    start_server();
    // keep alive
    pause();
#endif
    return 0;
}