#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif

typedef struct {
    int status;
    char **header_keys;
    char **header_vals;
    int header_count;
    unsigned char *body;
    size_t body_len;
} Response;

typedef Response (*RouteFunc)(const char* param);

typedef struct {
    char pattern[128]; // e.g., "/images/{name}" or "/images/<name>"
    RouteFunc func;
} RouteEntry;

static RouteEntry ROUTES[16];
static int ROUTE_COUNT = 0;

static void add_header(Response* r, const char* k, const char* v) {
    r->header_keys = (char**)realloc(r->header_keys, sizeof(char*) * (r->header_count + 1));
    r->header_vals = (char**)realloc(r->header_vals, sizeof(char*) * (r->header_count + 1));
    r->header_keys[r->header_count] = strdup(k);
    r->header_vals[r->header_count] = strdup(v);
    r->header_count++;
}

static Response make_response(int status, const char* content_type, const unsigned char* data, size_t len) {
    Response r;
    r.status = status;
    r.header_keys = NULL;
    r.header_vals = NULL;
    r.header_count = 0;
    r.body = NULL;
    r.body_len = 0;
    if (content_type) add_header(&r, "Content-Type", content_type);
    if (data && len > 0) {
        r.body = (unsigned char*)malloc(len);
        memcpy(r.body, data, len);
        r.body_len = len;
        char clen[32];
        snprintf(clen, sizeof(clen), "%zu", len);
        add_header(&r, "Content-Length", clen);
    }
    return r;
}

static void free_response(Response* r) {
    for (int i = 0; i < r->header_count; ++i) {
        free(r->header_keys[i]);
        free(r->header_vals[i]);
    }
    free(r->header_keys);
    free(r->header_vals);
    free(r->body);
}

static int register_route(const char* pattern, RouteFunc func) {
    if (ROUTE_COUNT < (int)(sizeof(ROUTES)/sizeof(ROUTES[0]))) {
        strncpy(ROUTES[ROUTE_COUNT].pattern, pattern, sizeof(ROUTES[ROUTE_COUNT].pattern)-1);
        ROUTES[ROUTE_COUNT].pattern[sizeof(ROUTES[ROUTE_COUNT].pattern)-1] = '\0';
        ROUTES[ROUTE_COUNT].func = func;
        ROUTE_COUNT++;
        return 1;
    }
    return 0;
}

#define ROUTE(PATH, NAME) \
    Response NAME(const char* param); \
    static int __reg_##NAME = (register_route(PATH, NAME), 1); \
    Response NAME(const char* param)

static int is_safe_filename(const char* s) {
    if (!s || !*s) return 0;
    size_t n = strlen(s);
    if (n > 255) return 0;
    if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0) return 0;
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static const char* guess_mime(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";
    if (strcasecmp(ext, ".png") == 0) return "image/png";
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcasecmp(ext, ".gif") == 0) return "image/gif";
    if (strcasecmp(ext, ".webp") == 0) return "image/webp";
    return "application/octet-stream";
}

static Response send_file_if_exists(const char* filename) {
    if (!is_safe_filename(filename)) {
        const char* msg = "Bad filename";
        return make_response(400, "text/plain", (const unsigned char*)msg, strlen(msg));
    }
    char path[512];
    snprintf(path, sizeof(path), "images/%s", filename);

    // prevent directory traversal by ensuring no '/' in filename (already ensured),
    // and only allow images directory prefix.
    FILE* f = fopen(path, "rb");
    if (!f) {
        const char* msg = "Not Found";
        return make_response(404, "text/plain", (const unsigned char*)msg, strlen(msg));
    }
    unsigned char* buf = NULL;
    size_t cap = 0, len = 0;
    unsigned char tmp[4096];
    size_t r;
    while ((r = fread(tmp, 1, sizeof(tmp), f)) > 0) {
        if (len + r > cap) {
            size_t newcap = cap ? cap * 2 : 8192;
            while (newcap < len + r) newcap *= 2;
            unsigned char* nbuf = (unsigned char*)realloc(buf, newcap);
            if (!nbuf) { fclose(f); free(buf); const char* msg="IO Error"; return make_response(500, "text/plain", (const unsigned char*)msg, strlen(msg)); }
            buf = nbuf; cap = newcap;
        }
        memcpy(buf + len, tmp, r);
        len += r;
    }
    fclose(f);
    Response resp = make_response(200, guess_mime(filename), buf, len);
    free(buf); // make_response copied it
    return resp;
}

static Response serve_request(const char* path) {
    // Support pattern "/images/{name}" or "/images/<name>"
    for (int i = 0; i < ROUTE_COUNT; ++i) {
        const char* pat = ROUTES[i].pattern;
        const char* placeholder_start = strstr(pat, "{name}");
        const char* placeholder_start2 = strstr(pat, "<name>");
        int matched = 0;
        const char* base = NULL;
        if (placeholder_start) {
            // treat as "/images/" prefix
            base = "/images/";
            matched = 1;
        } else if (placeholder_start2) {
            base = "/images/";
            matched = 1;
        }
        if (matched) {
            size_t base_len = strlen(base);
            if (strncmp(path, base, base_len) == 0) {
                const char* rest = path + base_len;
                if (strchr(rest, '/') == NULL) {
                    return ROUTES[i].func(rest);
                }
            }
        }
    }
    const char* msg = "Not Found";
    return make_response(404, "text/plain", (const unsigned char*)msg, strlen(msg));
}

// "Decorator" registration
ROUTE("/images/<name>", image_handler) {
    return send_file_if_exists(param);
}

static void write_bytes(const char* path, const unsigned char* data, size_t len) {
    // create directory "images" if not exists
    MKDIR("images");
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(data, 1, len, f);
    fclose(f);
}

int main(void) {
    MKDIR("images");
    write_bytes("images/hello.png", (const unsigned char*)"\x89PNG", 4);
    write_bytes("images/cat.jpg", (const unsigned char*)"\xFF\xD8\xFF", 3);
    write_bytes("images/dots..png", (const unsigned char*)"\x01\x02\x03\x04", 4);

    const char* tests[5] = {
        "/images/hello.png",
        "/images/missing.png",
        "/images/../../secret.txt",
        "/images/dots..png",
        "/images/CAT.JPG"
    };
    for (int i = 0; i < 5; ++i) {
        Response r = serve_request(tests[i]);
        printf("%s -> status=%d, len=%zu\n", tests[i], r.status, r.body_len);
        free_response(&r);
    }
    return 0;
}