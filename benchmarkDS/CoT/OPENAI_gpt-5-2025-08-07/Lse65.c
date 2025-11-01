// Minimal C HTTP server for PDF upload to ./uploads (POSIX only)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define PORT 8083
#define MAX_UPLOAD_SIZE (10*1024*1024) // 10MB
#define UPLOAD_DIR "uploads"

typedef struct {
    char* key;
    char* value;
} header_t;

typedef struct {
    header_t* headers;
    size_t header_count;
    unsigned char* data;
    size_t data_len;
} part_t;

static char* str_tolower(char* s) {
    for (char* p = s; *p; ++p) *p = (char)tolower((unsigned char)*p);
    return s;
}

static char* sanitize_filename(const char* input) {
    const char* base = input;
    for (const char* p = input; *p; ++p) {
        if (*p == '\\' || *p == '/') base = p + 1;
    }
    size_t n = strlen(base);
    if (n == 0) {
        char* out = strdup("file.pdf");
        return out;
    }
    char* out = (char*)malloc(n + 1);
    for (size_t i = 0; i < n; ++i) {
        char c = base[i];
        if (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-') out[i] = c;
        else out[i] = '_';
    }
    out[n] = '\0';
    if (out[0] == '.') out[0] = '_';
    return out;
}

static int is_pdf_magic(const unsigned char* data, size_t len) {
    size_t i = 0;
    while (i < len && (data[i] == 0x20 || data[i] == 0x09 || data[i] == 0x0D || data[i] == 0x0A)) i++;
    if (i + 5 > len) return 0;
    return data[i] == '%' && data[i+1] == 'P' && data[i+2] == 'D' && data[i+3] == 'F' && data[i+4] == '-';
}

static char* random_suffix(size_t n) {
    static const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char* s = (char*)malloc(n + 1);
    for (size_t i = 0; i < n; ++i) {
        s[i] = chars[rand() % 62];
    }
    s[n] = '\0';
    return s;
}

static char* ensure_unique(const char* base) {
    struct stat st;
    char* stem = NULL;
    char* ext = NULL;
    size_t blen = strlen(base);
    if (blen >= 4 && strncasecmp(base + blen - 4, ".pdf", 4) == 0) {
        stem = strndup(base, blen - 4);
        ext = strdup(base + blen - 4);
    } else {
        stem = strdup(base);
        ext = strdup("");
    }
    char* name = strdup(base);
    for (int i = 0; i < 1000; ++i) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", UPLOAD_DIR, name);
        if (stat(path, &st) != 0) { // not exists
            free(stem); free(ext);
            return name;
        }
        free(name);
        char* suf = random_suffix(6);
        size_t newlen = strlen(stem) + 1 + strlen(suf) + strlen(ext) + 1;
        name = (char*)malloc(newlen);
        snprintf(name, newlen, "%s-%s%s", stem, suf, ext);
        free(suf);
    }
    free(stem); free(ext);
    return name;
}

static int mkdir_p(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) return S_ISDIR(st.st_mode) ? 0 : -1;
    return mkdir(path, 0700);
}

static void send_response(int client, int code, const char* contentType, const char* body) {
    const char* status = "OK";
    switch (code) {
        case 200: status = "OK"; break;
        case 400: status = "Bad Request"; break;
        case 404: status = "Not Found"; break;
        case 405: status = "Method Not Allowed"; break;
        case 413: status = "Payload Too Large"; break;
        case 415: status = "Unsupported Media Type"; break;
        case 500: status = "Internal Server Error"; break;
    }
    char header[1024];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "X-Frame-Options: DENY\r\n"
        "Referrer-Policy: no-referrer\r\n"
        "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; form-action 'self'\r\n"
        "Cache-Control: no-store\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        code, status, contentType, strlen(body));
    send(client, header, strlen(header), 0);
    send(client, body, strlen(body), 0);
}

static part_t* parse_multipart(const unsigned char* body, size_t blen, const char* boundary, size_t* out_count) {
    *out_count = 0;
    part_t* parts = NULL;
    char* delim = NULL;
    char* delimCRLF = NULL;
    char* closeDelim = NULL;
    asprintf(&delim, "--%s", boundary);
    asprintf(&delimCRLF, "\r\n--%s", boundary);
    asprintf(&closeDelim, "--%s--", boundary);

    size_t pos = 0;
    // find initial delimiter
    int found = 0;
    for (size_t i = 0; i + strlen(delim) <= blen; ++i) {
        if (memcmp(body + i, delim, strlen(delim)) == 0) { pos = i + strlen(delim); found = 1; break; }
    }
    if (!found) goto done;
    if (pos + 2 <= blen && body[pos] == '\r' && body[pos+1] == '\n') pos += 2;

    while (1) {
        // headers end
        size_t headerEnd = blen;
        for (size_t i = pos; i + 4 <= blen; ++i) {
            if (body[i] == '\r' && body[i+1] == '\n' && body[i+2] == '\r' && body[i+3] == '\n') {
                headerEnd = i;
                break;
            }
        }
        if (headerEnd == blen) break;
        char* headerBlob = strndup((const char*)body + pos, headerEnd - pos);

        // parse headers
        part_t p;
        memset(&p, 0, sizeof(p));
        size_t hcap = 4;
        p.headers = (header_t*)calloc(hcap, sizeof(header_t));
        char* saveptr;
        char* line = strtok_r(headerBlob, "\r\n", &saveptr);
        while (line) {
            char* colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char* k = line;
                char* v = colon + 1;
                while (*v == ' ' || *v == '\t') v++;
                p.headers[p.header_count].key = strdup(k);
                for (char* c = p.headers[p.header_count].key; *c; ++c) *c = (char)tolower((unsigned char)*c);
                p.headers[p.header_count].value = strdup(v);
                p.header_count++;
                if (p.header_count >= hcap) {
                    hcap *= 2;
                    p.headers = (header_t*)realloc(p.headers, hcap * sizeof(header_t));
                }
            }
            line = strtok_r(NULL, "\r\n", &saveptr);
        }
        free(headerBlob);
        size_t dataStart = headerEnd + 4;

        // find next boundary
        size_t nextBoundary = blen, nextClose = blen;
        int hasBoundary = 0, isLast = 0;
        for (size_t i = dataStart; i + strlen(delimCRLF) <= blen; ++i) {
            if (memcmp(body + i, delimCRLF, strlen(delimCRLF)) == 0) { nextBoundary = i; hasBoundary = 1; break; }
        }
        for (size_t i = dataStart; i + 2 + strlen(closeDelim) <= blen; ++i) {
            if (memcmp(body + i, "\r\n", 2) == 0 && memcmp(body + i + 2, closeDelim, strlen(closeDelim)) == 0) {
                nextClose = i;
                isLast = 1;
                break;
            }
        }
        size_t dataEnd;
        if (isLast && (!hasBoundary || nextClose < nextBoundary)) {
            dataEnd = nextClose;
        } else if (hasBoundary) {
            dataEnd = nextBoundary;
        } else {
            // try close without preceding CRLF
            size_t nb = blen;
            for (size_t i = dataStart; i + strlen(closeDelim) <= blen; ++i) {
                if (memcmp(body + i, closeDelim, strlen(closeDelim)) == 0) { nb = i; break; }
            }
            if (nb == blen) break;
            dataEnd = nb - 2;
            isLast = 1;
        }
        if (dataEnd < dataStart) dataEnd = dataStart;

        p.data_len = dataEnd - dataStart;
        p.data = (unsigned char*)malloc(p.data_len);
        memcpy(p.data, body + dataStart, p.data_len);

        parts = (part_t*)realloc(parts, ((*out_count) + 1) * sizeof(part_t));
        parts[*out_count] = p;
        (*out_count)++;

        if (isLast) break;
        pos = nextBoundary + 2 + strlen(boundary);
        if (pos + 2 <= blen && body[pos] == '\r' && body[pos+1] == '\n') pos += 2;
    }

done:
    free(delim); free(delimCRLF); free(closeDelim);
    return parts;
}

static void free_parts(part_t* parts, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = 0; j < parts[i].header_count; ++j) {
            free(parts[i].headers[j].key);
            free(parts[i].headers[j].value);
        }
        free(parts[i].headers);
        free(parts[i].data);
    }
    free(parts);
}

static void handle_client(int client) {
    // Read headers
    char buffer[8192];
    char* req = NULL;
    size_t req_len = 0;
    while (1) {
        ssize_t r = recv(client, buffer, sizeof(buffer), MSG_PEEK);
        if (r <= 0) { break; }
        req = (char*)realloc(req, req_len + r);
        memcpy(req + req_len, buffer, r);
        req_len += r;
        char* p = strstr(req, "\r\n\r\n");
        if (p) {
            size_t header_size = (p - req) + 4;
            size_t consumed = 0;
            while (consumed < header_size) {
                ssize_t rr = recv(client, buffer, header_size - consumed > sizeof(buffer) ? sizeof(buffer) : header_size - consumed, 0);
                if (rr <= 0) break;
                consumed += rr;
            }
            break;
        } else {
            recv(client, buffer, r, 0);
        }
    }
    if (!req) { close(client); return; }

    // Parse request line
    char* line_end = strstr(req, "\r\n");
    if (!line_end) { send_response(client, 400, "text/plain; charset=utf-8", "Bad Request"); free(req); close(client); return; }
    *line_end = '\0';
    char method[8] = {0}, path[256] = {0};
    sscanf(req, "%7s %255s", method, path);

    // Headers
    char* headers_start = line_end + 2;
    char* headers_end = strstr(headers_start, "\r\n\r\n");
    size_t headers_len = headers_end ? (size_t)(headers_end - headers_start) : 0;
    char* headers_blob = strndup(headers_start, headers_len);

    // Extract headers
    char* content_type = NULL;
    size_t content_length = 0;
    {
        char* saveptr;
        char* line = strtok_r(headers_blob, "\r\n", &saveptr);
        while (line) {
            char* colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char* k = line;
                char* v = colon + 1;
                while (*v == ' ' || *v == '\t') v++;
                if (strcasecmp(k, "Content-Type") == 0) content_type = strdup(v);
                if (strcasecmp(k, "Content-Length") == 0) content_length = (size_t)strtoul(v, NULL, 10);
            }
            line = strtok_r(NULL, "\r\n", &saveptr);
        }
    }

    // Read body
    unsigned char* body = NULL;
    if (content_length > 0) {
        if (content_length > MAX_UPLOAD_SIZE) {
            send_response(client, 413, "text/plain; charset=utf-8", "Payload Too Large");
            goto cleanup;
        }
        body = (unsigned char*)malloc(content_length);
        size_t read_total = 0;
        while (read_total < content_length) {
            ssize_t r = recv(client, buffer, sizeof(buffer) < (ssize_t)(content_length - read_total) ? sizeof(buffer) : (ssize_t)(content_length - read_total), 0);
            if (r <= 0) break;
            memcpy(body + read_total, buffer, r);
            read_total += r;
        }
        if (read_total != content_length) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request");
            goto cleanup;
        }
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        const char* html =
            "<!doctype html><html lang='en'><head><meta charset='utf-8'><title>Upload PDF</title></head>"
            "<body><h1>Upload a PDF</h1>"
            "<form method='POST' action='/upload' enctype='multipart/form-data'>"
            "<input type='file' name='file' accept='application/pdf,.pdf' required>"
            "<button type='submit'>Upload</button>"
            "</form></body></html>";
        send_response(client, 200, "text/html; charset=utf-8", html);
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/upload") == 0) {
        if (!content_type || strcasestr(content_type, "multipart/form-data") == NULL) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: expected multipart/form-data");
            goto cleanup;
        }
        char* bpos = strcasestr(content_type, "boundary=");
        if (!bpos) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: missing boundary");
            goto cleanup;
        }
        bpos += 9;
        char boundary[256] = {0};
        size_t bl = 0;
        if (*bpos == '"' || *bpos == '\'') {
            char q = *bpos++;
            while (*bpos && *bpos != q && bl < sizeof(boundary)-1) boundary[bl++] = *bpos++;
        } else {
            while (*bpos && *bpos != ';' && bl < sizeof(boundary)-1) boundary[bl++] = *bpos++;
        }
        boundary[bl] = '\0';
        size_t part_count = 0;
        part_t* parts = parse_multipart(body, content_length, boundary, &part_count);
        part_t* filePart = NULL;
        char* filename = NULL;
        for (size_t i = 0; i < part_count; ++i) {
            for (size_t j = 0; j < parts[i].header_count; ++j) {
                if (strcasecmp(parts[i].headers[j].key, "content-disposition") == 0) {
                    char* cd = parts[i].headers[j].value;
                    if (strstr(cd, "name=\"file\"") && strstr(cd, "filename=")) {
                        char* fpos = strcasestr(cd, "filename=");
                        if (fpos) {
                            fpos += 9;
                            if (*fpos == '"' || *fpos == '\'') {
                                char q = *fpos++;
                                char* end = strchr(fpos, q);
                                if (end) filename = strndup(fpos, end - fpos);
                            } else {
                                char* end = strchr(fpos, ';');
                                if (!end) end = fpos + strlen(fpos);
                                filename = strndup(fpos, end - fpos);
                            }
                        }
                        filePart = &parts[i];
                        break;
                    }
                }
            }
            if (filePart) break;
        }
        if (!filePart || !filename) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: file field missing");
            free_parts(parts, part_count);
            goto cleanup;
        }
        char* safe = sanitize_filename(filename);
        size_t sl = strlen(safe);
        if (sl < 4 || strcasecmp(safe + sl - 4, ".pdf") != 0) {
            send_response(client, 415, "text/plain; charset=utf-8", "Unsupported Media Type: only .pdf allowed");
            free(safe); free(filename);
            free_parts(parts, part_count);
            goto cleanup;
        }
        if (filePart->data_len == 0) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: empty file");
            free(safe); free(filename);
            free_parts(parts, part_count);
            goto cleanup;
        }
        if (!is_pdf_magic(filePart->data, filePart->data_len)) {
            send_response(client, 415, "text/plain; charset=utf-8", "Unsupported Media Type: invalid PDF signature");
            free(safe); free(filename);
            free_parts(parts, part_count);
            goto cleanup;
        }
        mkdir_p(UPLOAD_DIR);
        char* unique = ensure_unique(safe);
        char pathbuf[1024];
        snprintf(pathbuf, sizeof(pathbuf), "%s/%s", UPLOAD_DIR, unique);
        FILE* fp = fopen(pathbuf, "wb");
        if (!fp) {
            send_response(client, 500, "text/plain; charset=utf-8", "Server Error: could not save file");
            free(unique); free(safe); free(filename);
            free_parts(parts, part_count);
            goto cleanup;
        }
        fwrite(filePart->data, 1, filePart->data_len, fp);
        fclose(fp);
        char msg[512];
        snprintf(msg, sizeof(msg), "Uploaded as: %s", unique);
        send_response(client, 200, "text/plain; charset=utf-8", msg);
        free(unique); free(safe); free(filename);
        free_parts(parts, part_count);
    } else {
        send_response(client, 404, "text/plain; charset=utf-8", "Not Found");
    }

cleanup:
    if (body) free(body);
    if (content_type) free(content_type);
    if (headers_blob) free(headers_blob);
    if (req) free(req);
    close(client);
}

static void run_tests() {
    printf("Running C tests...\n");
    // 1) sanitize traversal
    char* t1 = sanitize_filename("../etc/passwd");
    if (strstr(t1, "..") || strchr(t1, '/') || strchr(t1, '\\')) { fprintf(stderr, "sanitize 1 failed\n"); exit(1); }
    free(t1);
    // 2) sanitize hidden
    char* t2 = sanitize_filename(".hidden.pdf");
    if (t2[0] == '.') { fprintf(stderr, "sanitize 2 failed\n"); exit(1); }
    free(t2);
    // 3) pdf magic true
    unsigned char d1[] = "%PDF-1.7";
    if (!is_pdf_magic(d1, sizeof(d1)-1)) { fprintf(stderr, "pdf magic 3 failed\n"); exit(1); }
    // 4) pdf magic leading whitespace
    unsigned char d2[] = "\r\n %PDF-1.4";
    if (!is_pdf_magic(d2, sizeof(d2)-1)) { fprintf(stderr, "pdf magic 4 failed\n"); exit(1); }
    // 5) pdf magic false
    unsigned char d3[] = "no pdf";
    if (is_pdf_magic(d3, sizeof(d3)-1)) { fprintf(stderr, "pdf magic 5 failed\n"); exit(1); }
    printf("All C tests passed.\n");
}

int main() {
    srand((unsigned int)time(NULL));
    run_tests();

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) != 0) { perror("bind"); close(server); return 1; }
    if (listen(server, 10) != 0) { perror("listen"); close(server); return 1; }
    printf("C server running at http://127.0.0.1:%d/\n", PORT);
    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int client = accept(server, (struct sockaddr*)&caddr, &clen);
        if (client < 0) continue;
        handle_client(client);
    }
    close(server);
    return 0;
}