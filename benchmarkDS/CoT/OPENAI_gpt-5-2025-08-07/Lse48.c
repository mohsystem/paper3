/*
Chain-of-Through process in code generation:
1) Problem understanding: Implement img(fileParam) returning binary contents for a safe filename (as /img?file= would).
2) Security requirements: Prevent path traversal; restrict filename to safe chars; enforce base directory; robust I/O.
3) Secure coding generation: Validate filename, construct path safely, read with "rb", manage memory carefully.
4) Code review: No raw user path joins; checks for invalid chars and separators; errors return empty result.
5) Secure code output: Final code with tests.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
  #include <direct.h>
  #define MKDIR(dir) _mkdir(dir)
  #define PATH_SEP '\\'
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #define MKDIR(dir) mkdir(dir, 0700)
  #define PATH_SEP '/'
#endif

#define BASE_DIR "static"
#define MAX_NAME_LEN 255
#define MAX_PATH_LEN 1024

typedef struct {
    unsigned char* data;
    size_t size;
} ByteBuffer;

static int is_safe_name(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > MAX_NAME_LEN) return 0;
    if ((len == 1 && name[0] == '.') || (len == 2 && name[0] == '.' && name[1] == '.')) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

// img() returns ByteBuffer with contents on success; on failure, data=NULL,size=0
ByteBuffer img(const char* fileParam) {
    ByteBuffer result = { NULL, 0 };
    if (!is_safe_name(fileParam)) {
        return result;
    }

    char path[MAX_PATH_LEN];
    int n = snprintf(path, sizeof(path), "%s%c%s", BASE_DIR, PATH_SEP, fileParam);
    if (n <= 0 || (size_t)n >= sizeof(path)) {
        return result;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        return result;
    }

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return result; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return result; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return result; }

    result.data = (unsigned char*)malloc((size_t)sz);
    if (!result.data) { fclose(f); return result; }
    result.size = (size_t)sz;

    size_t rd = fread(result.data, 1, result.size, f);
    fclose(f);
    if (rd != result.size) {
        free(result.data);
        result.data = NULL;
        result.size = 0;
    }
    return result;
}

static void ensure_base_and_fixtures() {
    MKDIR(BASE_DIR);

    char path1[MAX_PATH_LEN];
    snprintf(path1, sizeof(path1), "%s%c%s", BASE_DIR, PATH_SEP, "index.txt");
    FILE* f1 = fopen(path1, "rb");
    if (!f1) {
        f1 = fopen(path1, "wb");
        if (f1) {
            const char* msg = "Hello from index\n";
            fwrite(msg, 1, strlen(msg), f1);
            fclose(f1);
        }
    } else {
        fclose(f1);
    }

    char path2[MAX_PATH_LEN];
    snprintf(path2, sizeof(path2), "%s%c%s", BASE_DIR, PATH_SEP, "image.bin");
    FILE* f2 = fopen(path2, "rb");
    if (!f2) {
        f2 = fopen(path2, "wb");
        if (f2) {
            for (int i = 0; i < 16; ++i) {
                unsigned char b = (unsigned char)((i * 7) & 0xFF);
                fwrite(&b, 1, 1, f2);
            }
            fclose(f2);
        }
    } else {
        fclose(f2);
    }
}

static void print_result(const char* label, const ByteBuffer* buf) {
    printf("%s -> size=%zu, head=[", label, buf->size);
    size_t show = buf->size < 8 ? buf->size : 8;
    for (size_t i = 0; i < show; ++i) {
        if (i) printf(", ");
        printf("%d", (int)buf->data[i]);
    }
    printf("]\n");
}

int main(void) {
    ensure_base_and_fixtures();

    // 5 test cases
    ByteBuffer r1 = img("index.txt");           // valid
    ByteBuffer r2 = img("image.bin");           // valid
    ByteBuffer r3 = img("../etc/passwd");       // traversal attempt -> empty
    ByteBuffer r4 = img("bad$name.txt");        // invalid chars -> empty
    ByteBuffer r5 = img("missing.txt");         // non-existent -> empty

    print_result("TC1 index.txt", &r1);
    print_result("TC2 image.bin", &r2);
    print_result("TC3 ../etc/passwd", &r3);
    print_result("TC4 bad$name.txt", &r4);
    print_result("TC5 missing.txt", &r5);

    free(r1.data);
    free(r2.data);
    free(r3.data);
    free(r4.data);
    free(r5.data);

    return 0;
}