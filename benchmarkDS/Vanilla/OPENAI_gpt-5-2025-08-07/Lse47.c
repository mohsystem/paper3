#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static const char* content_type_for(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return "application/octet-stream";
    if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcasecmp(dot, ".png") == 0) return "image/png";
    if (strcasecmp(dot, ".gif") == 0) return "image/gif";
    if (strcasecmp(dot, ".bmp") == 0) return "image/bmp";
    if (strcasecmp(dot, ".webp") == 0) return "image/webp";
    return "application/octet-stream";
}

// Loads an image from images/ folder into outData buffer, outLen, and sets *outContentType.
// Returns 1 if file exists and loaded, 0 otherwise. Caller must free(*outData) if 1 is returned.
int load_image_from_images(const char* filename, unsigned char** outData, size_t* outLen, const char** outContentType) {
    if (!filename || !outData || !outLen || !outContentType) return 0;
    if (strstr(filename, "..")) return 0;

    char path[4096];
    snprintf(path, sizeof(path), "images/%s", filename);
    if (path[0] == '/' || path[0] == '\\') return 0;

    if (!file_exists(path)) return 0;

    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return 0; }
    rewind(f);
    unsigned char* buf = NULL;
    if (len > 0) {
        buf = (unsigned char*)malloc((size_t)len);
        if (!buf) { fclose(f); return 0; }
        size_t rd = fread(buf, 1, (size_t)len, f);
        if (rd != (size_t)len) { free(buf); fclose(f); return 0; }
    } else {
        buf = NULL;
    }
    fclose(f);
    *outData = buf;
    *outLen = (size_t)len;
    *outContentType = content_type_for(filename);
    return 1;
}

int main() {
    const char* tests[5] = {"sample.jpg", "logo.png", "missing.png", "test.gif", "photos/pic.jpg"};
    for (int i = 0; i < 5; ++i) {
        const char* t = tests[i];
        unsigned char* data = NULL;
        size_t len = 0;
        const char* ctype = NULL;
        int ok = load_image_from_images(t, &data, &len, &ctype);
        printf("Test '%s': exists=%s", t, ok ? "true" : "false");
        if (ok) {
            printf(", bytes=%zu, type=%s", len, ctype);
            free(data);
        }
        printf("\n");
    }
    return 0;
}