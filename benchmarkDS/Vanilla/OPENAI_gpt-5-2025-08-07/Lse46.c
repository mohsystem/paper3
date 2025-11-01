#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif

// img() opens images/name and returns a malloc'ed buffer with file content.
// The size is returned via out_len. Caller must free() the returned buffer.
unsigned char* img(const char* name, size_t* out_len) {
    char path[1024];
    snprintf(path, sizeof(path), "images/%s", name);
    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    *out_len = n;
    return buf;
}

static void ensure_images_dir() {
#ifdef _WIN32
    _mkdir("images");
#else
    mkdir("images", 0755);
#endif
}

static void write_file(const char* name, const char* content) {
    char path[1024];
    snprintf(path, sizeof(path), "images/%s", name);
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(content, 1, strlen(content), f);
    fclose(f);
}

int main(void) {
    ensure_images_dir();
    write_file("t1.txt", "alpha");
    write_file("t2.txt", "bravo charlie");
    write_file("t3.txt", "delta echo foxtrot");
    write_file("t4.txt", "golf hotel india juliet");
    write_file("t5.txt", "kilo lima mike november oscar");

    const char* tests[5] = {"t1.txt", "t2.txt", "t3.txt", "t4.txt", "t5.txt"};
    for (int i = 0; i < 5; ++i) {
        size_t len = 0;
        unsigned char* data = img(tests[i], &len);
        if (!data) {
            printf("Failed to read images/%s\n", tests[i]);
        } else {
            printf("Read %zu bytes from images/%s: ", len, tests[i]);
            size_t preview = len < 16 ? len : 16;
            for (size_t j = 0; j < preview; ++j) {
                unsigned char c = data[j];
                if (c >= 32 && c <= 126) putchar((int)c);
                else putchar('.');
            }
            putchar('\n');
            free(data);
        }
    }
    return 0;
}