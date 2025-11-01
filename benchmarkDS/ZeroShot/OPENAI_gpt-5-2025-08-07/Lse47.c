#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

typedef struct {
    int status;
    char contentType[64];
    unsigned char* body;
    size_t body_len;
} HttpResult;

typedef HttpResult (*RouteFunc)(const char* filename);

typedef struct {
    const char* path;
    RouteFunc fn;
} RouteEntry;

static int is_allowed_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return 0;
    char ext[16];
    size_t n = strlen(dot);
    if (n >= sizeof(ext)) return 0;
    for (size_t i = 0; i < n; i++) ext[i] = (char)tolower((unsigned char)dot[i]);
    ext[n] = '\0';
    return strcmp(ext, ".png") == 0 ||
           strcmp(ext, ".jpg") == 0 ||
           strcmp(ext, ".jpeg") == 0 ||
           strcmp(ext, ".gif") == 0 ||
           strcmp(ext, ".webp") == 0;
}

static int is_safe_filename(const char* filename) {
    // Only allow base filenames with [A-Za-z0-9._-], no slashes, no backslashes
    if (!filename || !*filename) return 0;
    for (const char* p = filename; *p; ++p) {
        char c = *p;
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    if (strchr(filename, '/')) return 0;
    if (strchr(filename, '\\')) return 0;
    if (strncmp(filename, "..", 2) == 0) return 0;
    return 1;
}

static const char* guess_content_type(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return "application/octet-stream";
    char ext[16];
    size_t n = strlen(dot);
    if (n >= sizeof(ext)) return "application/octet-stream";
    for (size_t i = 0; i < n; i++) ext[i] = (char)tolower((unsigned char)dot[i]);
    ext[n] = '\0';
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".webp") == 0) return "image/webp";
    return "application/octet-stream";
}

HttpResult serve_image(const char* filename) {
    HttpResult res;
    res.status = 500;
    strcpy(res.contentType, "text/plain; charset=utf-8");
    res.body = NULL;
    res.body_len = 0;

    if (!filename || !*filename) {
        res.status = 400;
        res.body = (unsigned char*)strdup("Bad Request");
        res.body_len = strlen((char*)res.body);
        return res;
    }
    if (!is_safe_filename(filename)) {
        res.status = 400;
        res.body = (unsigned char*)strdup("Bad Request");
        res.body_len = strlen((char*)res.body);
        return res;
    }
    if (!is_allowed_extension(filename)) {
        res.status = 403;
        res.body = (unsigned char*)strdup("Forbidden");
        res.body_len = strlen((char*)res.body);
        return res;
    }

    char path[512];
    snprintf(path, sizeof(path), "images/%s", filename);

    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        res.status = 404;
        res.body = (unsigned char*)strdup("Not Found");
        res.body_len = strlen((char*)res.body);
        return res;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        res.status = 500;
        res.body = (unsigned char*)strdup("Internal Server Error");
        res.body_len = strlen((char*)res.body);
        return res;
    }
    unsigned char* buf = (unsigned char*)malloc((size_t)st.st_size);
    if (!buf) {
        fclose(f);
        res.status = 500;
        res.body = (unsigned char*)strdup("Internal Server Error");
        res.body_len = strlen((char*)res.body);
        return res;
    }
    size_t rd = fread(buf, 1, (size_t)st.st_size, f);
    fclose(f);
    if (rd != (size_t)st.st_size) {
        free(buf);
        res.status = 500;
        res.body = (unsigned char*)strdup("Internal Server Error");
        res.body_len = strlen((char*)res.body);
        return res;
    }

    res.status = 200;
    strncpy(res.contentType, guess_content_type(filename), sizeof(res.contentType) - 1);
    res.contentType[sizeof(res.contentType) - 1] = '\0';
    res.body = buf;
    res.body_len = rd;
    return res;
}

HttpResult handle_request(const char* url) {
    const char* prefix = "/images/";
    size_t plen = strlen(prefix);
    if (strncmp(url, prefix, plen) == 0) {
        const char* filename = url + plen;
        return serve_image(filename);
    }
    HttpResult res;
    res.status = 404;
    strcpy(res.contentType, "text/plain; charset=utf-8");
    res.body = (unsigned char*)strdup("Not Found");
    res.body_len = strlen((char*)res.body);
    return res;
}

static void ensure_test_images() {
    // create images directory if not exists
#ifdef _WIN32
    _mkdir("images");
#else
    mkdir("images", 0700);
#endif
    FILE* f = fopen("images/sample.png", "rb");
    if (!f) {
        f = fopen("images/sample.png", "wb");
        if (f) { unsigned char d[8] = {0x89, 'P', 'N', 'G', 0, 0, 0, 0}; fwrite(d, 1, 8, f); fclose(f); }
    } else fclose(f);
    f = fopen("images/photo.jpg", "rb");
    if (!f) {
        f = fopen("images/photo.jpg", "wb");
        if (f) { const char* d = "JPEGDATA"; fwrite(d, 1, 8, f); fclose(f); }
    } else fclose(f);
}

int main() {
    ensure_test_images();
    // 5 test cases
    HttpResult r1 = handle_request("/images/sample.png"); printf("%d\n", r1.status);
    HttpResult r2 = handle_request("/images/none.png");   printf("%d\n", r2.status);
    HttpResult r3 = handle_request("/images/../secret");  printf("%d\n", r3.status);
    HttpResult r4 = handle_request("/images/bad.txt");    printf("%d\n", r4.status);
    HttpResult r5 = handle_request("/images/photo.jpg");  printf("%d\n", r5.status);
    // free allocated bodies
    if (r1.body) free(r1.body);
    if (r2.body) free(r2.body);
    if (r3.body) free(r3.body);
    if (r4.body) free(r4.body);
    if (r5.body) free(r5.body);
    return 0;
}