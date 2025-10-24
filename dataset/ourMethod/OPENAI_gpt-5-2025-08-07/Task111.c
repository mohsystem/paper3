#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_HEADER 4096
#define MAX_BODY 8192
#define MAX_STR 1024

typedef struct {
    char bind_ip[16];
    uint16_t port;
} ServerConfigC;

typedef struct {
    volatile int *stop_flag;
    ServerConfigC cfg;
} ServerArgs;

static void msleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void str_to_lower(char *s) {
    for (; *s; ++s) {
        if (*s >= 'A' && *s <= 'Z') *s = (char)(*s - 'A' + 'a');
    }
}

static char *trim_dup(const char *s) {
    const char *a = s;
    while (*a == ' ' || *a == '\t' || *a == '\r' || *a == '\n') a++;
    const char *b = s + strlen(s);
    while (b > a && (b[-1] == ' ' || b[-1] == '\t' || b[-1] == '\r' || b[-1] == '\n')) b--;
    size_t n = (size_t)(b - a);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, a, n);
    out[n] = '\0';
    return out;
}

static bool ascii_printable(const char *s, size_t maxlen) {
    size_t n = strlen(s);
    if (n > maxlen) return false;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x09) return false;
        if (c == 0x0B || c == 0x0C) return false;
        if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') return false;
    }
    return true;
}

static bool parse_int64(const char *s, int64_t *out) {
    char *t = trim_dup(s);
    if (!t) return false;
    size_t len = strlen(t);
    if (len == 0 || len > 32) {
        free(t);
        return false;
    }
    int sign = 1;
    size_t i = 0;
    if (t[0] == '+' || t[0] == '-') {
        sign = (t[0] == '-') ? -1 : 1;
        i++;
    }
    if (i >= len) {
        free(t);
        return false;
    }
    int64_t val = 0;
    for (; i < len; ++i) {
        char c = t[i];
        if (c < '0' || c > '9') {
            free(t);
            return false;
        }
        int d = c - '0';
        if (val > (INT64_MAX - d) / 10) {
            free(t);
            return false;
        }
        val = val * 10 + d;
    }
    *out = val * sign;
    free(t);
    return true;
}

static char *xml_escape(const char *in, size_t maxlen) {
    size_t n = strlen(in);
    if (n > maxlen) n = maxlen;
    // Worst-case each char expands to 6 chars (&quot;)
    size_t cap = n * 6 + 1;
    char *out = (char *)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        char ch = in[i];
        const char *rep = NULL;
        switch (ch) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&apos;"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rl = strlen(rep);
            if (j + rl >= cap) { free(out); return NULL; }
            memcpy(out + j, rep, rl);
            j += rl;
        } else {
            if (j + 1 >= cap) { free(out); return NULL; }
            out[j++] = ch;
        }
    }
    out[j] = '\0';
    return out;
}

static char *http_build_response(const char *xml, int status, const char *status_text) {
    size_t xml_len = strlen(xml);
    char header[256];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/xml; charset=utf-8\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "Connection: close\r\n"
                     "Content-Length: %zu\r\n\r\n",
                     status, status_text, xml_len);
    if (n <= 0 || (size_t)n >= sizeof(header)) return NULL;
    size_t total = (size_t)n + xml_len;
    char *resp = (char *)malloc(total + 1);
    if (!resp) return NULL;
    memcpy(resp, header, (size_t)n);
    memcpy(resp + n, xml, xml_len);
    resp[total] = '\0';
    return resp;
}

static char *xml_fault(int code, const char *msg) {
    char codebuf[32];
    snprintf(codebuf, sizeof(codebuf), "%d", code);
    char *msg_esc = xml_escape(msg, 256);
    if (!msg_esc) return NULL;
    const char *pre = "<?xml version=\"1.0\"?><methodResponse><fault><value><struct>"
                      "<member><name>faultCode</name><value><int>";
    const char *mid = "</int></value></member>"
                      "<member><name>faultString</name><value><string>";
    const char *suf = "</string></value></member></struct></value></fault></methodResponse>";
    size_t total = strlen(pre) + strlen(codebuf) + strlen(mid) + strlen(msg_esc) + strlen(suf);
    char *out = (char *)malloc(total + 1);
    if (!out) { free(msg_esc); return NULL; }
    snprintf(out, total + 1, "%s%s%s%s%s", pre, codebuf, mid, msg_esc, suf);
    free(msg_esc);
    return out;
}

static char *xml_int_response(int64_t v) {
    char num[64];
    snprintf(num, sizeof(num), "%" PRId64, v);
    const char *pre = "<?xml version=\"1.0\"?><methodResponse><params><param><value><int>";
    const char *suf = "</int></value></param></params></methodResponse>";
    size_t total = strlen(pre) + strlen(num) + strlen(suf);
    char *out = (char *)malloc(total + 1);
    if (!out) return NULL;
    snprintf(out, total + 1, "%s%s%s", pre, num, suf);
    return out;
}

static char *xml_string_response(const char *s) {
    char *esc = xml_escape(s, MAX_STR);
    if (!esc) return NULL;
    const char *pre = "<?xml version=\"1.0\"?><methodResponse><params><param><value><string>";
    const char *suf = "</string></value></param></params></methodResponse>";
    size_t total = strlen(pre) + strlen(esc) + strlen(suf);
    char *out = (char *)malloc(total + 1);
    if (!out) { free(esc); return NULL; }
    snprintf(out, total + 1, "%s%s%s", pre, esc, suf);
    free(esc);
    return out;
}

static bool between_tags(const char *body, const char *tag, char **out) {
    size_t open_len = strlen(tag) + 2;
    size_t close_len = strlen(tag) + 3;
    char open[64], close[64];
    if (open_len >= sizeof(open) || close_len >= sizeof(close)) return false;
    snprintf(open, sizeof(open), "<%s>", tag);
    snprintf(close, sizeof(close), "</%s>", tag);
    char *a = strstr((char *)body, open);
    if (!a) return false;
    a += strlen(open);
    char *b = strstr(a, close);
    if (!b) return false;
    size_t n = (size_t)(b - a);
    char *s = (char *)malloc(n + 1);
    if (!s) return false;
    memcpy(s, a, n);
    s[n] = '\0';
    *out = s;
    return true;
}

typedef enum { VAL_NONE = 0, VAL_INT = 1, VAL_STRING = 2 } ValType;

typedef struct {
    ValType type;
    int64_t i;
    char *s;
} XmlVal;

static void xmlval_free(XmlVal *v) {
    if (v->type == VAL_STRING && v->s) free(v->s);
    v->type = VAL_NONE;
    v->i = 0;
    v->s = NULL;
}

static bool parse_param_value(const char *param_xml, XmlVal *out) {
    memset(out, 0, sizeof(*out));
    char *value_sec = NULL;
    if (!between_tags(param_xml, "value", &value_sec)) return false;
    char *vtrim = trim_dup(value_sec);
    free(value_sec);
    if (!vtrim) return false;

    char *int_content = NULL;
    if (between_tags(vtrim, "int", &int_content) || between_tags(vtrim, "i4", &int_content)) {
        int64_t iv = 0;
        bool ok = parse_int64(int_content, &iv);
        free(int_content);
        free(vtrim);
        if (!ok) return false;
        out->type = VAL_INT;
        out->i = iv;
        return true;
    }
    char *str_content = NULL;
    if (between_tags(vtrim, "string", &str_content)) {
        bool ok = ascii_printable(str_content, MAX_STR);
        if (!ok) { free(str_content); free(vtrim); return false; }
        out->type = VAL_STRING;
        out->s = str_content;
        free(vtrim);
        return true;
    }
    // Bare text
    if (strlen(vtrim) > 0) {
        bool ok = ascii_printable(vtrim, MAX_STR);
        if (!ok) { free(vtrim); return false; }
        out->type = VAL_STRING;
        out->s = vtrim;
        return true;
    }
    free(vtrim);
    return false;
}

static bool valid_method_name(const char *name) {
    size_t n = strlen(name);
    if (n == 0 || n > 32) return false;
    for (size_t i = 0; i < n; ++i) {
        char c = name[i];
        if (!(c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return false;
    }
    return true;
}

static char *handle_xmlrpc_request(const char *xml, volatile int *stop_flag) {
    if (strstr(xml, "<!ENTITY") || strstr(xml, "<!DOCTYPE")) {
        return xml_fault(400, "External entities and DTDs are not allowed.");
    }
    char *methodNameSec = NULL;
    if (!between_tags(xml, "methodName", &methodNameSec)) {
        return xml_fault(400, "Missing methodName");
    }
    char *mtrim = trim_dup(methodNameSec);
    free(methodNameSec);
    if (!mtrim) return xml_fault(500, "Internal error");
    if (!valid_method_name(mtrim)) {
        free(mtrim);
        return xml_fault(400, "Invalid methodName");
    }
    // Extract params block (optional)
    char *params_block = NULL;
    (void)between_tags(xml, "params", &params_block);

    // Parse params
    XmlVal params[8];
    int pcount = 0;
    for (int i = 0; i < 8; ++i) { params[i].type = VAL_NONE; params[i].i = 0; params[i].s = NULL; }

    if (params_block) {
        char *search = params_block;
        const char *open = "<param>";
        const char *close = "</param>";
        while (pcount < 8) {
            char *a = strstr(search, open);
            if (!a) break;
            a += strlen(open);
            char *b = strstr(a, close);
            if (!b) break;
            size_t n = (size_t)(b - a);
            char *p = (char *)malloc(n + 1);
            if (!p) break;
            memcpy(p, a, n);
            p[n] = '\0';
            XmlVal v;
            if (!parse_param_value(p, &v)) {
                free(p);
                free(params_block);
                free(mtrim);
                for (int i = 0; i < pcount; ++i) xmlval_free(&params[i]);
                return xml_fault(400, "Invalid param");
            }
            free(p);
            params[pcount++] = v;
            search = b + strlen(close);
        }
        free(params_block);
    }

    // Dispatch
    char *resp = NULL;
    if (strcmp(mtrim, "add") == 0) {
        if (pcount != 2 || params[0].type != VAL_INT || params[1].type != VAL_INT) {
            resp = xml_fault(400, "add expects two integers");
        } else {
            int64_t r = params[0].i + params[1].i;
            resp = xml_int_response(r);
        }
    } else if (strcmp(mtrim, "multiply") == 0) {
        if (pcount != 2 || params[0].type != VAL_INT || params[1].type != VAL_INT) {
            resp = xml_fault(400, "multiply expects two integers");
        } else {
            int64_t r = params[0].i * params[1].i;
            resp = xml_int_response(r);
        }
    } else if (strcmp(mtrim, "echo") == 0) {
        if (pcount != 1 || params[0].type != VAL_STRING) {
            resp = xml_fault(400, "echo expects one string");
        } else {
            resp = xml_string_response(params[0].s);
        }
    } else if (strcmp(mtrim, "strlen") == 0) {
        if (pcount != 1 || params[0].type != VAL_STRING) {
            resp = xml_fault(400, "strlen expects one string");
        } else {
            resp = xml_int_response((int64_t)strlen(params[0].s));
        }
    } else if (strcmp(mtrim, "shutdown") == 0) {
        if (pcount != 0) {
            resp = xml_fault(400, "shutdown expects no params");
        } else {
            *stop_flag = 1;
            resp = xml_string_response("ok");
        }
    } else {
        resp = xml_fault(404, "Unknown method");
    }

    for (int i = 0; i < pcount; ++i) xmlval_free(&params[i]);
    free(mtrim);
    return resp;
}

static bool read_until_delim(int fd, char **out, const char *delim, size_t max_bytes) {
    size_t cap = 1024;
    size_t len = 0;
    char *buf = (char *)malloc(cap);
    if (!buf) return false;
    while (len < max_bytes) {
        char tmp[512];
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n < 0) {
            free(buf);
            return false;
        }
        if (n == 0) break;
        if (len + (size_t)n >= cap) {
            size_t ncap = cap * 2;
            while (ncap <= len + (size_t)n) ncap *= 2;
            char *nb = (char *)realloc(buf, ncap);
            if (!nb) { free(buf); return false; }
            buf = nb; cap = ncap;
        }
        memcpy(buf + len, tmp, (size_t)n);
        len += (size_t)n;
        buf[len] = '\0';
        if (strstr(buf, delim)) {
            *out = buf;
            return true;
        }
    }
    buf[len] = '\0';
    bool found = strstr(buf, delim) != NULL;
    if (!found) free(buf);
    else *out = buf;
    return found;
}

static bool parse_content_length_hdr(const char *headers, size_t *cl_out) {
    *cl_out = 0;
    const char *p = headers;
    while (*p) {
        const char *end = strstr(p, "\r\n");
        size_t len = end ? (size_t)(end - p) : strlen(p);
        if (len == 0) break;
        char line[256];
        size_t cpy = len < sizeof(line) - 1 ? len : sizeof(line) - 1;
        memcpy(line, p, cpy);
        line[cpy] = '\0';
        char head[32];
        snprintf(head, sizeof(head), "content-length:");
        char lowline[256];
        strncpy(lowline, line, sizeof(lowline) - 1);
        lowline[sizeof(lowline) - 1] = '\0';
        str_to_lower(lowline);
        if (strncmp(lowline, head, strlen(head)) == 0) {
            const char *v = line + strlen("Content-Length:");
            while (*v == ' ' || *v == '\t') v++;
            int64_t cl = 0;
            if (!parse_int64(v, &cl) || cl < 0 || cl > MAX_BODY) return false;
            *cl_out = (size_t)cl;
            return true;
        }
        if (!end) break;
        p = end + 2;
    }
    return false;
}

static bool read_http_request(int fd, char **body_out) {
    *body_out = NULL;
    char *headers = NULL;
    if (!read_until_delim(fd, &headers, "\r\n\r\n", MAX_HEADER)) return false;
    char *sep = strstr(headers, "\r\n\r\n");
    if (!sep) { free(headers); return false; }
    size_t header_len = (size_t)(sep - headers);
    char *header_part = (char *)malloc(header_len + 3);
    if (!header_part) { free(headers); return false; }
    memcpy(header_part, headers, header_len);
    header_part[header_len] = '\r';
    header_part[header_len + 1] = '\n';
    header_part[header_len + 2] = '\0';
    size_t cl = 0;
    bool ok = parse_content_length_hdr(header_part, &cl);
    free(header_part);
    if (!ok) { free(headers); return false; }
    // Remainder after header
    char *remainder = sep + 4;
    size_t rem_len = strlen(remainder);
    char *body = (char *)malloc(cl + 1);
    if (!body) { free(headers); return false; }
    size_t copied = rem_len > cl ? cl : rem_len;
    memcpy(body, remainder, copied);
    size_t need = cl - copied;
    size_t offset = copied;
    while (need > 0 && offset < MAX_BODY) {
        char tmp[1024];
        ssize_t n = recv(fd, tmp, (need > sizeof(tmp) ? sizeof(tmp) : need), 0);
        if (n <= 0) { free(headers); free(body); return false; }
        memcpy(body + offset, tmp, (size_t)n);
        offset += (size_t)n;
        if ((size_t)n > need) break;
        need -= (size_t)n;
    }
    body[cl] = '\0';
    free(headers);
    if (strlen(body) != cl) { free(body); return false; }
    if (!ascii_printable(body, MAX_BODY)) { free(body); return false; }
    *body_out = body;
    return true;
}

static bool send_all(int fd, const char *data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, data + off, len - off, 0);
        if (n < 0) return false;
        if (n == 0) break;
        off += (size_t)n;
    }
    return off == len;
}

static void *server_thread(void *arg) {
    ServerArgs *sa = (ServerArgs *)arg;
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) pthread_exit(NULL);
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct timeval tv;
    tv.tv_sec = 10; tv.tv_usec = 0;
    setsockopt(srv, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(sa->cfg.port);
    inet_pton(AF_INET, sa->cfg.bind_ip, &addr.sin_addr);
    if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) { close(srv); pthread_exit(NULL); }
    if (listen(srv, 8) < 0) { close(srv); pthread_exit(NULL); }

    while (!*(sa->stop_flag)) {
        struct sockaddr_in cli;
        socklen_t cl = sizeof(cli);
        int cfd = accept(srv, (struct sockaddr *)&cli, &cl);
        if (cfd < 0) continue;
        setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        char *body = NULL;
        bool ok = read_http_request(cfd, &body);
        char *xml_resp = NULL;
        if (!ok) {
            xml_resp = xml_fault(400, "Bad Request");
            char *resp = http_build_response(xml_resp ? xml_resp : "", 400, "Bad Request");
            if (resp) { send_all(cfd, resp, strlen(resp)); free(resp); }
            if (xml_resp) free(xml_resp);
            close(cfd);
            continue;
        }
        xml_resp = handle_xmlrpc_request(body, sa->stop_flag);
        free(body);
        if (!xml_resp) xml_resp = xml_fault(500, "Internal error");
        char *resp = http_build_response(xml_resp, 200, "OK");
        if (resp) { send_all(cfd, resp, strlen(resp)); free(resp); }
        free(xml_resp);
        close(cfd);
    }
    close(srv);
    pthread_exit(NULL);
}

static bool client_post_xml(const char *host, uint16_t port, const char *xml, char **resp_body_out) {
    *resp_body_out = NULL;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;
    struct timeval tv;
    tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) { close(fd); return false; }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { close(fd); return false; }
    char header[256];
    int n = snprintf(header, sizeof(header),
                     "POST /RPC2 HTTP/1.1\r\nHost: %s:%u\r\nContent-Type: text/xml\r\nConnection: close\r\nContent-Length: %zu\r\n\r\n",
                     host, port, strlen(xml));
    if (n <= 0 || (size_t)n >= sizeof(header)) { close(fd); return false; }
    if (!send_all(fd, header, (size_t)n)) { close(fd); return false; }
    if (!send_all(fd, xml, strlen(xml))) { close(fd); return false; }
    // Read headers
    char *headers = NULL;
    if (!read_until_delim(fd, &headers, "\r\n\r\n", 8192)) { close(fd); return false; }
    char *sep = strstr(headers, "\r\n\r\n");
    if (!sep) { free(headers); close(fd); return false; }
    size_t header_len = (size_t)(sep - headers);
    char *header_part = (char *)malloc(header_len + 3);
    if (!header_part) { free(headers); close(fd); return false; }
    memcpy(header_part, headers, header_len);
    header_part[header_len] = '\r'; header_part[header_len + 1] = '\n'; header_part[header_len + 2] = '\0';
    size_t cl = 0;
    bool ok = parse_content_length_hdr(header_part, &cl);
    free(header_part);
    if (!ok) { free(headers); close(fd); return false; }
    char *remainder = sep + 4;
    size_t rem_len = strlen(remainder);
    char *body = (char *)malloc(cl + 1);
    if (!body) { free(headers); close(fd); return false; }
    size_t copied = rem_len > cl ? cl : rem_len;
    memcpy(body, remainder, copied);
    size_t need = cl - copied;
    size_t off = copied;
    free(headers);
    while (need > 0 && off < 65536) {
        char tmp[1024];
        ssize_t r = recv(fd, tmp, (need > sizeof(tmp) ? sizeof(tmp) : need), 0);
        if (r <= 0) { free(body); close(fd); return false; }
        memcpy(body + off, tmp, (size_t)r);
        off += (size_t)r;
        if ((size_t)r > need) break;
        need -= (size_t)r;
    }
    body[cl] = '\0';
    close(fd);
    if (strlen(body) != cl) { free(body); return false; }
    *resp_body_out = body;
    return true;
}

int main(void) {
    volatile int stop_flag = 0;
    ServerArgs sa;
    memset(&sa, 0, sizeof(sa));
    sa.stop_flag = &stop_flag;
    snprintf(sa.cfg.bind_ip, sizeof(sa.cfg.bind_ip), "%s", "127.0.0.1");
    sa.cfg.port = 8090;

    pthread_t tid;
    if (pthread_create(&tid, NULL, server_thread, &sa) != 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    msleep(200);

    char *resp = NULL;

    // Test 1: add
    const char *add_xml =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>add</methodName><params>"
        "<param><value><int>10</int></value></param>"
        "<param><value><int>20</int></value></param>"
        "</params></methodCall>";
    if (client_post_xml("127.0.0.1", sa.cfg.port, add_xml, &resp)) {
        printf("add response: %s\n", resp);
        free(resp);
    } else {
        printf("add request failed\n");
    }

    // Test 2: multiply
    const char *mul_xml =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>multiply</methodName><params>"
        "<param><value><int>3</int></value></param>"
        "<param><value><int>7</int></value></param>"
        "</params></methodCall>";
    if (client_post_xml("127.0.0.1", sa.cfg.port, mul_xml, &resp)) {
        printf("multiply response: %s\n", resp);
        free(resp);
    } else {
        printf("multiply request failed\n");
    }

    // Test 3: echo
    const char *echo_xml =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>echo</methodName><params>"
        "<param><value><string>Hello, C XML-RPC</string></value></param>"
        "</params></methodCall>";
    if (client_post_xml("127.0.0.1", sa.cfg.port, echo_xml, &resp)) {
        printf("echo response: %s\n", resp);
        free(resp);
    } else {
        printf("echo request failed\n");
    }

    // Test 4: strlen
    const char *strlen_xml =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>strlen</methodName><params>"
        "<param><value><string>ABCDE</string></value></param>"
        "</params></methodCall>";
    if (client_post_xml("127.0.0.1", sa.cfg.port, strlen_xml, &resp)) {
        printf("strlen response: %s\n", resp);
        free(resp);
    } else {
        printf("strlen request failed\n");
    }

    // Test 5: shutdown
    const char *shutdown_xml =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>shutdown</methodName></methodCall>";
    if (client_post_xml("127.0.0.1", sa.cfg.port, shutdown_xml, &resp)) {
        printf("shutdown response: %s\n", resp);
        free(resp);
    } else {
        printf("shutdown request failed\n");
    }

    pthread_join(tid, NULL);
    return 0;
}