#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

static void ensure_uploads_dir() {
#ifdef _WIN32
    _mkdir("uploads");
#else
    mkdir("uploads", 0755);
#endif
}

static char* sanitize_filename(const char* name) {
    // Return newly allocated sanitized name
    const char* last_slash = strrchr(name, '/');
    const char* last_back = strrchr(name, '\\');
    const char* base = name;
    if (last_slash && last_back)
        base = (last_slash > last_back) ? last_slash + 1 : last_back + 1;
    else if (last_slash)
        base = last_slash + 1;
    else if (last_back)
        base = last_back + 1;

    size_t len = strlen(base);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        if (base[i] == '.' && i + 1 < len && base[i + 1] == '.') {
            // skip '..'
            continue;
        }
        out[j++] = base[i];
    }
    if (j == 0) {
        free(out);
        out = (char*)malloc(12);
        strcpy(out, "upload.bin");
        return out;
    }
    out[j] = '\0';
    return out;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len) {
            int hi = hexval(s[i+1]);
            int lo = hexval(s[i+2]);
            if (hi >= 0 && lo >= 0) {
                out[j++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                out[j++] = s[i];
            }
        } else if (s[i] == '+') {
            out[j++] = ' ';
        } else {
            out[j++] = s[i];
        }
    }
    out[j] = '\0';
    return out;
}

// The upload_file function: saves data under uploads directory. Returns newly allocated absolute path string (caller must free).
char* upload_file(const char* filename, const unsigned char* data, size_t len) {
    ensure_uploads_dir();
    char* safe = sanitize_filename(filename);
    if (!safe) return NULL;

    // Build relative path "uploads/safe"
    size_t path_len = strlen("uploads/") + strlen(safe) + 1;
    char* path = (char*)malloc(path_len);
    if (!path) { free(safe); return NULL; }
    snprintf(path, path_len, "uploads/%s", safe);

    FILE* f = fopen(path, "wb");
    if (!f) { free(safe); free(path); return NULL; }
    fwrite(data, 1, len, f);
    fclose(f);

    // Get absolute path
    char* abs = realpath(path, NULL);
    if (!abs) {
        // If realpath fails, return relative
        abs = strdup(path);
    }

    free(safe);
    free(path);
    return abs;
}

struct client_args {
    int fd;
};

static void* handle_client(void* arg) {
    int client_fd = ((struct client_args*)arg)->fd;
    free(arg);

    char* header = NULL;
    size_t header_cap = 0;
    size_t header_len = 0;

    char buf[4096];
    int found = 0;
    while (!found) {
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) { close(client_fd); return NULL; }
        if (header_len + (size_t)n + 1 > header_cap) {
            header_cap = (header_len + n + 1) * 2;
            header = (char*)realloc(header, header_cap);
            if (!header) { close(client_fd); return NULL; }
        }
        memcpy(header + header_len, buf, (size_t)n);
        header_len += (size_t)n;
        header[header_len] = '\0';
        char* p = strstr(header, "\r\n\r\n");
        if (p) {
            found = 1;
        }
        if (header_len > (1u << 20)) { close(client_fd); free(header); return NULL; }
    }

    char* header_end = strstr(header, "\r\n\r\n");
    size_t head_len = (size_t)(header_end - header);
    size_t remain_len = header_len - head_len - 4;
    char* remain = header_end + 4;

    // Parse request line
    char* line_end = strstr(header, "\r\n");
    if (!line_end) { close(client_fd); free(header); return NULL; }
    size_t line_len = (size_t)(line_end - header);
    char* reqline = (char*)malloc(line_len + 1);
    memcpy(reqline, header, line_len);
    reqline[line_len] = '\0';

    char method[8], path[1024], version[16];
    method[0]=path[0]=version[0]='\0';
    sscanf(reqline, "%7s %1023s %15s", method, path, version);

    if (strcmp(method, "POST") != 0) {
        const char* resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, resp, strlen(resp), 0);
        close(client_fd);
        free(reqline);
        free(header);
        return NULL;
    }

    // Content-Length
    long content_length = -1;
    {
        const char* cl_key = "Content-Length:";
        char* pos = strcasestr(header, cl_key);
        if (!pos) {
            const char* resp = "HTTP/1.1 411 Length Required\r\nContent-Length: 0\r\n\r\n";
            send(client_fd, resp, strlen(resp), 0);
            close(client_fd);
            free(reqline);
            free(header);
            return NULL;
        }
        char* line_end2 = strstr(pos, "\r\n");
        char tmp[64]; memset(tmp, 0, sizeof(tmp));
        if (line_end2) {
            size_t l = (size_t)(line_end2 - pos);
            if (l >= sizeof(tmp)) l = sizeof(tmp) - 1;
            memcpy(tmp, pos, l);
            char* colon = strchr(tmp, ':');
            if (colon) {
                content_length = strtol(colon + 1, NULL, 10);
            }
        }
    }

    // Parse filename from query
    char* filename = NULL;
    {
        char* qmark = strchr(path, '?');
        if (qmark) {
            char* query = qmark + 1;
            const char* key = "filename=";
            char* kpos = strstr(query, key);
            if (kpos) {
                char* vstart = kpos + (int)strlen(key);
                char* vend = strchr(vstart, '&');
                size_t vlen = vend ? (size_t)(vend - vstart) : strlen(vstart);
                char* raw = (char*)malloc(vlen + 1);
                memcpy(raw, vstart, vlen);
                raw[vlen] = '\0';
                char* dec = url_decode(raw);
                free(raw);
                filename = dec;
            }
        }
    }

    if (!filename || filename[0] == '\0') {
        const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 29\r\n\r\nMissing filename parameter";
        send(client_fd, resp, strlen(resp), 0);
        close(client_fd);
        free(filename);
        free(reqline);
        free(header);
        return NULL;
    }

    // Read body
    size_t body_len = (size_t)((content_length > 0) ? content_length : 0);
    unsigned char* body = (unsigned char*)malloc(body_len);
    size_t copied = 0;
    if (remain_len > 0) {
        size_t to_copy = remain_len > body_len ? body_len : remain_len;
        memcpy(body, remain, to_copy);
        copied = to_copy;
    }
    while (copied < body_len) {
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        size_t to_copy = (size_t)n;
        if (copied + to_copy > body_len) to_copy = body_len - copied;
        memcpy(body + copied, buf, to_copy);
        copied += to_copy;
    }

    char* saved_path = upload_file(filename, body, body_len);
    char respmsg[2048];
    if (saved_path) {
        snprintf(respmsg, sizeof(respmsg), "Saved: %s\n", saved_path);
    } else {
        snprintf(respmsg, sizeof(respmsg), "Failed to save file\n");
    }
    char header_resp[256];
    snprintf(header_resp, sizeof(header_resp), "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\n\r\n", strlen(respmsg));
    send(client_fd, header_resp, strlen(header_resp), 0);
    send(client_fd, respmsg, strlen(respmsg), 0);

    close(client_fd);
    free(saved_path);
    free(body);
    free(filename);
    free(reqline);
    free(header);

    return NULL;
}

static void* server_thread(void* arg) {
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return NULL;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8083);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(server_fd);
        return NULL;
    }
    if (listen(server_fd, 16) != 0) {
        close(server_fd);
        return NULL;
    }

    while (1) {
        int cfd = accept(server_fd, NULL, NULL);
        if (cfd < 0) break;
        struct client_args* ca = (struct client_args*)malloc(sizeof(struct client_args));
        ca->fd = cfd;
        pthread_t t;
        pthread_create(&t, NULL, handle_client, ca);
        pthread_detach(t);
    }

    close(server_fd);
    return NULL;
}

int main() {
    // Start server on port 8083
    pthread_t srv;
    pthread_create(&srv, NULL, server_thread, NULL);
    pthread_detach(srv);

    // 5 test cases calling upload_file directly
    char* p1 = upload_file("hello.txt", (const unsigned char*)"Hello, C!\n", 10);
    char* p2 = upload_file("data.bin", (const unsigned char*)"\x00\x01\x02\x03\x04\x05", 6);
    char* p3 = upload_file("image.jpg", (const unsigned char*)"\xFF\xD8\x00\x01", 4);
    char* p4 = upload_file("../evil/../../should_not_escape.txt", (const unsigned char*)"safe", 4);
    char* p5 = upload_file("unicode_файл.txt", (const unsigned char*)"Привет", strlen("Привет"));

    if (p1) { printf("%s\n", p1); free(p1); }
    if (p2) { printf("%s\n", p2); free(p2); }
    if (p3) { printf("%s\n", p3); free(p3); }
    if (p4) { printf("%s\n", p4); free(p4); }
    if (p5) { printf("%s\n", p5); free(p5); }

    return 0;
}