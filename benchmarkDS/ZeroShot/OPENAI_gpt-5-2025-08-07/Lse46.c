#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#define PATH_SEP "\\"
#else
#include <unistd.h>
#include <fcntl.h>
#define PATH_SEP "/"
#endif

typedef struct {
    int statusCode;            // 200, 400, 403, 404, 500
    char contentType[64];
    unsigned char* data;
    size_t data_len;
    char message[64];
} ImgResult;

static int is_allowed_name(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    if (name[0] == '.') return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static void guess_content_type(const char* name, char* out, size_t out_len) {
    const char* ext = strrchr(name, '.');
    if (!ext) {
        snprintf(out, out_len, "application/octet-stream");
        return;
    }
    if (strcasecmp(ext, ".png") == 0) { snprintf(out, out_len, "image/png"); return; }
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) { snprintf(out, out_len, "image/jpeg"); return; }
    if (strcasecmp(ext, ".gif") == 0) { snprintf(out, out_len, "image/gif"); return; }
    if (strcasecmp(ext, ".webp") == 0) { snprintf(out, out_len, "image/webp"); return; }
    if (strcasecmp(ext, ".svg") == 0) { snprintf(out, out_len, "image/svg+xml"); return; }
    if (strcasecmp(ext, ".txt") == 0) { snprintf(out, out_len, "text/plain; charset=utf-8"); return; }
    snprintf(out, out_len, "application/octet-stream");
}

static int make_images_dir() {
#ifdef _WIN32
    struct _stat st;
    if (_stat("images", &st) == 0) return 0;
    return _mkdir("images");
#else
    struct stat st;
    if (stat("images", &st) == 0) return 0;
    return mkdir("images", 0700);
#endif
}

ImgResult img(const char* name) {
    ImgResult res;
    res.statusCode = 500;
    snprintf(res.contentType, sizeof(res.contentType), "text/plain; charset=utf-8");
    res.data = NULL;
    res.data_len = 0;
    snprintf(res.message, sizeof(res.message), "Internal Server Error");

    if (!name || !is_allowed_name(name)) {
        res.statusCode = 400;
        snprintf(res.message, sizeof(res.message), "Invalid file name");
        return res;
    }

    if (make_images_dir() != 0) {
        res.statusCode = 500;
        snprintf(res.message, sizeof(res.message), "Internal Server Error");
        return res;
    }

    char path[512];
    snprintf(path, sizeof(path), "images%s%s", PATH_SEP, name);

#ifndef _WIN32
    // Prevent path traversal by resolving realpath and validating prefix
    char base_real[512];
    char path_real[512];
    if (!realpath("images", base_real)) {
        res.statusCode = 500;
        snprintf(res.message, sizeof(res.message), "Internal Server Error");
        return res;
    }
    char path_input[512];
    snprintf(path_input, sizeof(path_input), "%s", path);
    if (!realpath(path_input, path_real)) {
        // realpath fails if file doesn't exist; fallback to simple checks
        // Reject any attempt containing ".." just in case
        if (strstr(name, "..") != NULL) {
            res.statusCode = 400;
            snprintf(res.message, sizeof(res.message), "Invalid path");
            return res;
        }
    } else {
        size_t base_len = strlen(base_real);
        if (strncmp(path_real, base_real, base_len) != 0 || (path_real[base_len] != '/' && path_real[base_len] != '\0')) {
            res.statusCode = 400;
            snprintf(res.message, sizeof(res.message), "Invalid path");
            return res;
        }
    }
#endif

    // Open safely
    int fd = -1;
#ifdef _WIN32
    fd = _open(path, _O_BINARY | _O_RDONLY);
#else
#ifdef O_NOFOLLOW
    fd = open(path, O_RDONLY | O_NOFOLLOW);
    if (fd == -1 && errno == ELOOP) {
        res.statusCode = 403;
        snprintf(res.message, sizeof(res.message), "Forbidden");
        return res;
    }
#else
    fd = open(path, O_RDONLY);
#endif
#endif

    if (fd == -1) {
        res.statusCode = 404;
        snprintf(res.message, sizeof(res.message), "Not found");
        return res;
    }

    // Determine file size
#ifdef _WIN32
    struct _stat st;
    if (_fstat(fd, &st) != 0 || (st.st_mode & _S_IFDIR)) {
        _close(fd);
        res.statusCode = 404;
        snprintf(res.message, sizeof(res.message), "Not found");
        return res;
    }
    size_t size = (size_t)st.st_size;
#else
    struct stat st;
    if (fstat(fd, &st) != 0 || S_ISDIR(st.st_mode)) {
        close(fd);
        res.statusCode = 404;
        snprintf(res.message, sizeof(res.message), "Not found");
        return res;
    }
    size_t size = (size_t)st.st_size;
#endif

    unsigned char* buffer = (unsigned char*)malloc(size ? size : 1);
    if (!buffer) {
#ifdef _WIN32
        _close(fd);
#else
        close(fd);
#endif
        res.statusCode = 500;
        snprintf(res.message, sizeof(res.message), "Internal Server Error");
        return res;
    }

    size_t total = 0;
    while (total < size) {
#ifdef _WIN32
        int r = _read(fd, buffer + total, (unsigned int)(size - total));
#else
        ssize_t r = read(fd, buffer + total, size - total);
#endif
        if (r <= 0) break;
        total += (size_t)r;
    }

#ifdef _WIN32
    _close(fd);
#else
    close(fd);
#endif

    if (total != size) {
        free(buffer);
        res.statusCode = 500;
        snprintf(res.message, sizeof(res.message), "Internal Server Error");
        return res;
    }

    guess_content_type(name, res.contentType, sizeof(res.contentType));
    res.statusCode = 200;
    snprintf(res.message, sizeof(res.message), "OK");
    res.data = buffer;
    res.data_len = size;
    return res;
}

int main(void) {
    // Prepare test environment
#ifdef _WIN32
    _mkdir("images");
#else
    mkdir("images", 0700);
#endif
    FILE* f = fopen("images" PATH_SEP "hello.txt", "rb");
    if (!f) {
        f = fopen("images" PATH_SEP "hello.txt", "wb");
        if (f) {
            const char* s = "Hello world";
            fwrite(s, 1, strlen(s), f);
            fclose(f);
        }
    } else {
        fclose(f);
    }

    // 5 tests
    ImgResult r1 = img("hello.txt");     // 200
    ImgResult r2 = img("");              // 400
    ImgResult r3 = img("../secret");     // 400
    ImgResult r4 = img("bad/name.png");  // 400
    ImgResult r5 = img("nope.png");      // 404

    ImgResult* results[5] = { &r1, &r2, &r3, &r4, &r5 };
    const char* labels[5] = { "valid existing", "missing param", "path traversal", "invalid chars", "not found" };

    for (int i = 0; i < 5; ++i) {
        printf("%s -> status=%d, type=%s, bytes=%zu\n",
               labels[i], results[i]->statusCode, results[i]->contentType, results[i]->data_len);
        free(results[i]->data);
    }

    return 0;
}