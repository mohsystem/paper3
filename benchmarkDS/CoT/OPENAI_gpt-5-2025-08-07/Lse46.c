#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

static const char* ALLOWED_EXT[] = {"png","jpg","jpeg","gif","webp","bmp","svg"};
static int allowed_ext_count = 7;

static int ends_with_icase(const char* s, const char* suf) {
    size_t ls = strlen(s), lt = strlen(suf);
    if (lt > ls) return 0;
    s += ls - lt;
    for (size_t i = 0; i < lt; ++i) {
        char a = s[i];
        char b = suf[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
    }
    return 1;
}

static int is_valid_name(const char* name) {
    if (!name || !*name) return 0;
    if (strstr(name, "/") || strstr(name, "\\") || strstr(name, "..")) return 0;
    const char* dot = strrchr(name, '.');
    if (!dot || dot == name || *(dot+1) == '\0') return 0;
    const char* ext = dot + 1;
    int ok = 0;
    for (int i = 0; i < allowed_ext_count; ++i) {
        if (ends_with_icase(name, ALLOWED_EXT[i])) { ok = 1; break; }
    }
    return ok;
}

static const char* guess_mime(const char* name) {
    if (ends_with_icase(name, ".png")) return "image/png";
    if (ends_with_icase(name, ".jpg") || ends_with_icase(name, ".jpeg")) return "image/jpeg";
    if (ends_with_icase(name, ".gif")) return "image/gif";
    if (ends_with_icase(name, ".webp")) return "image/webp";
    if (ends_with_icase(name, ".bmp")) return "image/bmp";
    if (ends_with_icase(name, ".svg")) return "image/svg+xml";
    return "application/octet-stream";
}

// Function accepts input parameter name and returns a newly allocated HTTP-like response buffer.
// Caller must free(*out_buf).
static int img(const char* name, char** out_buf, size_t* out_len) {
    *out_buf = NULL;
    *out_len = 0;
    if (!is_valid_name(name)) {
        const char* msg = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        *out_len = strlen(msg);
        *out_buf = (char*)malloc(*out_len);
        if (*out_buf) memcpy(*out_buf, msg, *out_len);
        return 0;
    }
    char cwd[4096];
#ifdef _WIN32
    _getcwd(cwd, sizeof(cwd));
#else
    getcwd(cwd, sizeof(cwd));
#endif
    char path[8192];
    snprintf(path, sizeof(path), "%s/%s/%s", cwd, "images", name);

    FILE* f = fopen(path, "rb");
    if (!f) {
        const char* msg = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        *out_len = strlen(msg);
        *out_buf = (char*)malloc(*out_len);
        if (*out_buf) memcpy(*out_buf, msg, *out_len);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return -1; }
    fseek(f, 0, SEEK_SET);
    size_t bsz = (size_t)sz;
    char* body = (char*)malloc(bsz);
    if (!body) { fclose(f); return -1; }
    size_t rd = fread(body, 1, bsz, f);
    fclose(f);
    if (rd != bsz) { free(body); return -1; }

    const char* mime = guess_mime(name);
    char header[512];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                        mime, bsz);
    if (hlen <= 0) { free(body); return -1; }

    *out_len = (size_t)hlen + bsz;
    *out_buf = (char*)malloc(*out_len);
    if (!*out_buf) { free(body); return -1; }
    memcpy(*out_buf, header, (size_t)hlen);
    memcpy(*out_buf + hlen, body, bsz);
    free(body);
    return 0;
}

static void create_sample_files() {
#ifdef _WIN32
    _mkdir("images");
#else
    mkdir("images", 0755);
#endif
    // sample.png
    {
        FILE* f = fopen("images/sample.png", "rb");
        if (!f) {
            f = fopen("images/sample.png", "wb");
            if (f) {
                unsigned char png[] = {0x89,'P','N','G', 0x0D,0x0A,0x1A,0x0A, 0x00};
                fwrite(png, 1, sizeof(png), f);
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
    // logo.jpg
    {
        FILE* f = fopen("images/logo.jpg", "rb");
        if (!f) {
            f = fopen("images/logo.jpg", "wb");
            if (f) {
                unsigned char jpg[] = {0xFF,0xD8,0xFF, 0x00, 0x11, 0x22};
                fwrite(jpg, 1, sizeof(jpg), f);
                fclose(f);
            }
        } else {
            fclose(f);
        }
    }
}

int main() {
    create_sample_files();
    const char* tests[5] = {
        "sample.png",
        "logo.jpg",
        "../secret.png",
        "notfound.png",
        "subdir/evil.png"
    };
    for (int i = 0; i < 5; ++i) {
        char* resp = NULL;
        size_t len = 0;
        if (img(tests[i], &resp, &len) == 0 && resp) {
            // print first line and content-length
            char* crlf = strstr(resp, "\r\n");
            if (crlf) {
                size_t first_line_len = (size_t)(crlf - resp);
                char tmp[256];
                size_t copy = first_line_len < sizeof(tmp)-1 ? first_line_len : sizeof(tmp)-1;
                memcpy(tmp, resp, copy);
                tmp[copy] = '\0';
                // find Content-Length header
                char* cl = strstr(resp, "Content-Length:");
                char clen[64] = "N/A";
                if (cl) {
                    char* clend = strstr(cl, "\r\n");
                    if (clend) {
                        size_t l = (size_t)(clend - cl);
                        size_t copy2 = l < sizeof(clen)-1 ? l : sizeof(clen)-1;
                        memcpy(clen, cl, copy2);
                        clen[copy2] = '\0';
                    }
                }
                printf("Test name=%s -> %s, %s\n", tests[i], tmp, clen);
            } else {
                printf("Test name=%s -> response length=%zu\n", tests[i], len);
            }
            free(resp);
        } else {
            printf("Test name=%s -> error\n", tests[i]);
        }
    }
    return 0;
}