#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* allocate_and_describe(long long size) {
    if (size < 0) size = 0;
    size_t n = (size_t)size;

    unsigned char* buffer = NULL;
    if (n > 0) {
        buffer = (unsigned char*)calloc(n, 1); // zero-initialized
        if (!buffer) {
            char* empty = (char*)malloc(1);
            if (empty) empty[0] = '\0';
            return empty;
        }
    }

    size_t out_len = (n == 0) ? 0 : (n * 3 - 1);
    char* out = (char*)malloc(out_len + 1);
    if (!out) {
        if (buffer) free(buffer);
        return NULL;
    }

    char* p = out;
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) { *p++ = ' '; }
        sprintf(p, "%02X", buffer ? buffer[i] : 0);
        p += 2;
    }
    out[out_len] = '\0';

    if (buffer) free(buffer);
    return out;
}

int main(int argc, char** argv) {
    if (argc == 2) {
        long long v = atoll(argv[1]);
        char* s = allocate_and_describe(v);
        if (s) {
            printf("%s\n", s);
            free(s);
        }
    } else {
        long long tests[5] = {0, 1, 3, 8, 16};
        for (int i = 0; i < 5; ++i) {
            char* s = allocate_and_describe(tests[i]);
            if (s) {
                printf("%s\n", s);
                free(s);
            }
        }
    }
    return 0;
}