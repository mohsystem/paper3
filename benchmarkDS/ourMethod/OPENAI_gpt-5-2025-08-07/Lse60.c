#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_SIZE 1048576u /* 1 MiB cap */

static char* allocate_and_dump_buffer(size_t size) {
    if (size > MAX_SIZE) {
        return NULL;
    }
    /* Allocate zero-initialized buffer */
    unsigned char *buf = NULL;
    if (size == 0) {
        /* No need to allocate data buffer; just return empty string */
        char *out = (char*)malloc(1);
        if (!out) return NULL;
        out[0] = '\0';
        return out;
    }
    buf = (unsigned char*)calloc(size, 1);
    if (!buf) {
        return NULL;
    }

    /* Compute output length: "xx xx ...", which is size*3 - 1 characters */
    size_t out_len = size * 3u - 1u;
    char *out = (char*)malloc(out_len + 1u);
    if (!out) {
        free(buf);
        return NULL;
    }

    /* Build hex string */
    size_t pos = 0;
    for (size_t i = 0; i < size; i++) {
        unsigned int v = (unsigned int)buf[i];
        static const char hexdigits[] = "0123456789abcdef";
        out[pos++] = hexdigits[(v >> 4) & 0xF];
        out[pos++] = hexdigits[v & 0xF];
        if (i + 1 < size) {
            out[pos++] = ' ';
        }
    }
    out[pos] = '\0';

    free(buf);
    return out;
}

static int parse_size_arg(const char *s, size_t *out_size) {
    if (!s || !out_size) return -1;
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return -1;
    if (v > MAX_SIZE) return -1;
    *out_size = (size_t)v;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        size_t size = 0;
        if (parse_size_arg(argv[1], &size) != 0) {
            fprintf(stderr, "Error: size must be between 0 and %u\n", (unsigned)MAX_SIZE);
            return 1;
        }
        char *out = allocate_and_dump_buffer(size);
        if (!out) {
            fprintf(stderr, "Error: allocation or processing failed\n");
            return 1;
        }
        if (*out) {
            printf("%s\n", out);
        } else {
            printf("\n");
        }
        free(out);
    } else {
        size_t tests[5] = {0, 1, 8, 16, 64};
        for (size_t i = 0; i < 5; i++) {
            char *out = allocate_and_dump_buffer(tests[i]);
            if (!out) {
                fprintf(stderr, "Error: allocation or processing failed\n");
                return 1;
            }
            if (*out) {
                printf("%s\n", out);
            } else {
                printf("\n");
            }
            free(out);
        }
    }
    return 0;
}