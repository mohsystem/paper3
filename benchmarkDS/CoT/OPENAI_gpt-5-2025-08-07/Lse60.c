#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_SIZE ((size_t)1048576) /* 1 MiB upper bound for safety */

/* Allocates a zero-initialized buffer of the given size and returns its contents as a hex string.
   Caller must free the returned string. Returns NULL on error. */
char* allocate_and_get_hex(size_t size) {
    if (size > MAX_SIZE) {
        return NULL;
    }
    /* Check overflow for hex string length: 2 chars per byte + 1 for null */
    if (size > (SIZE_MAX - 1) / 2) {
        return NULL;
    }
    unsigned char *buffer = NULL;
    if (size > 0) {
        buffer = (unsigned char*)calloc(size, 1);
        if (!buffer) {
            return NULL;
        }
    }

    size_t out_len = size * 2 + 1;
    char *out = (char*)malloc(out_len);
    if (!out) {
        free(buffer);
        return NULL;
    }

    static const char HEX[] = "0123456789abcdef";
    char *p = out;
    for (size_t i = 0; i < size; ++i) {
        unsigned char b = buffer ? buffer[i] : 0;
        *p++ = HEX[(b >> 4) & 0xF];
        *p++ = HEX[b & 0xF];
    }
    *p = '\0';

    free(buffer);
    return out;
}

static int parse_size_arg(const char *s, size_t *out) {
    if (!s || !*s) return 0;
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    *out = (size_t)v;
    return 1;
}

int main(int argc, char **argv) {
    if (argc == 2) {
        size_t size = 0;
        if (!parse_size_arg(argv[1], &size)) {
            fprintf(stderr, "Invalid size argument.\n");
            return 1;
        }
        char *hex = allocate_and_get_hex(size);
        if (!hex) {
            fprintf(stderr, "Allocation or formatting failed.\n");
            return 1;
        }
        puts(hex);
        free(hex);
        return 0;
    }
    /* 5 test cases if no argument provided */
    size_t tests[5] = {0, 1, 8, 16, 32};
    for (int i = 0; i < 5; ++i) {
        char *hex = allocate_and_get_hex(tests[i]);
        if (!hex) {
            fprintf(stderr, "Test case failed for size %zu\n", tests[i]);
            continue;
        }
        puts(hex);
        free(hex);
    }
    return 0;
}