#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_BUFFER_SIZE ((size_t)1048576) /* 1 MiB */

static char* secure_dump_buffer(size_t size) {
    if (size > MAX_BUFFER_SIZE) {
        return NULL;
    }
    if (size == 0) {
        char *empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\0';
        }
        return empty;
    }

    /* Check for overflow: we allocate 3 * size + 1 bytes for the output string */
    if (size > (SIZE_MAX - 1) / 3) {
        return NULL;
    }
    size_t out_len = size * 3 + 1;
    char *out = (char*)malloc(out_len);
    if (!out) {
        return NULL;
    }

    uint8_t *buf = (uint8_t*)malloc(size);
    if (!buf) {
        free(out);
        return NULL;
    }
    for (size_t i = 0; i < size; ++i) {
        buf[i] = (uint8_t)(i & 0xFF); /* deterministic pattern */
    }

    static const char HEX[] = "0123456789ABCDEF";
    size_t pos = 0;
    for (size_t i = 0; i < size; ++i) {
        uint8_t v = buf[i];
        out[pos++] = HEX[(v >> 4) & 0x0F];
        out[pos++] = HEX[v & 0x0F];
        if ((i & 15) == 15) {
            out[pos++] = '\n';
        } else {
            out[pos++] = ' ';
        }
    }
    if ((size & 15) != 0 && pos > 0 && out[pos - 1] == ' ') {
        out[pos - 1] = '\n';
    }
    out[pos] = '\0';

    free(buf);
    return out;
}

static int parse_size_arg(const char *s, size_t *out_size) {
    if (!s || !*s) return -1;
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0) return -1;
    if (end == s || *end != '\0') return -1;
    if (v > SIZE_MAX) return -1;
    *out_size = (size_t)v;
    return 0;
}

static void run_single(size_t size) {
    if (size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error: Size too large (max %zu)\n", MAX_BUFFER_SIZE);
        exit(1);
    }
    char *dump = secure_dump_buffer(size);
    if (!dump && size != 0) {
        fprintf(stderr, "Error: Allocation failed or invalid size\n");
        exit(1);
    }
    if (dump) {
        fputs(dump, stdout);
        free(dump);
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        size_t val = 0;
        if (parse_size_arg(argv[1], &val) != 0) {
            fprintf(stderr, "Error: Invalid number: %s\n", argv[1]);
            return 1;
        }
        run_single(val);
        return 0;
    }

    /* 5 test cases when no argument is provided */
    size_t tests[5] = {0, 1, 16, 31, 64};
    for (size_t i = 0; i < 5; ++i) {
        printf("=== size %zu ===\n", tests[i]);
        run_single(tests[i]);
    }
    return 0;
}