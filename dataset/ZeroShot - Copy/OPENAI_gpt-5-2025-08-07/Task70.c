#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#endif

#define MAX_SIZE (10 * 1024 * 1024) /* 10MB */

static bool is_within_base(const char* base, const char* target) {
    size_t blen = strlen(base);
    if (strncmp(base, target, blen) != 0) {
        return false;
    }
    /* Ensure boundary: either exact match or next char is path separator */
    if (target[blen] == '\0') return true;
#ifdef _WIN32
    if (target[blen] == '\\' || target[blen] == '/') return true;
#else
    if (target[blen] == '/') return true;
#endif
    return false;
}

/* Securely reads a file within the current working directory, max 10MB, returns malloc'd string or NULL */
char* read_file_secure(const char* inputPath) {
    if (inputPath == NULL || *inputPath == '\0') {
        return NULL;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return NULL;
    }

    char baseResolved[PATH_MAX];
    if (realpath(cwd, baseResolved) == NULL) {
        return NULL;
    }

    char targetResolved[PATH_MAX];
    if (realpath(inputPath, targetResolved) == NULL) {
        return NULL;
    }

    if (!is_within_base(baseResolved, targetResolved)) {
        return NULL;
    }

    struct stat st;
    if (stat(targetResolved, &st) != 0) {
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        return NULL;
    }
    if (st.st_size < 0 || st.st_size > MAX_SIZE) {
        return NULL;
    }

    FILE* f = fopen(targetResolved, "rb");
    if (!f) {
        return NULL;
    }

    size_t size = (size_t)st.st_size;
    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t readBytes = fread(buf, 1, size, f);
    if (readBytes != size) {
        if (ferror(f)) {
            free(buf);
            fclose(f);
            return NULL;
        }
    }
    buf[readBytes] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            printf("----- %s -----\n", argv[i]);
            char* content = read_file_secure(argv[i]);
            if (content) {
                printf("%s\n", content);
                free(content);
            } else {
                printf("ERROR: Unable to read file securely.\n");
            }
        }
        return 0;
    }

    /* 5 test cases */
    FILE* f = fopen("c_t1.txt", "wb");
    if (f) {
        fputs("C Test 1: Hello World", f);
        fclose(f);
    }

#ifdef _WIN32
    _mkdir("c_subdir");
#else
    mkdir("c_subdir", 0700);
#endif

    f = fopen("c_subdir/c_t2.txt", "wb");
    if (f) {
        fputs("C Test 2: Inside subdir", f);
        fclose(f);
    }

    const char* tests[5] = {
        "c_t1.txt",              /* valid */
        "c_subdir/c_t2.txt",     /* valid in subdir */
        "c_no_such.txt",         /* non-existent */
        "c_subdir",              /* directory */
        "../c_outside.txt"       /* escape attempt */
    };

    for (int i = 0; i < 5; ++i) {
        printf("===== Testing: %s =====\n", tests[i]);
        char* content = read_file_secure(tests[i]);
        if (content) {
            printf("%s\n", content);
            free(content);
        } else {
            printf("ERROR: Unable to read file securely.\n");
        }
    }

    return 0;
}