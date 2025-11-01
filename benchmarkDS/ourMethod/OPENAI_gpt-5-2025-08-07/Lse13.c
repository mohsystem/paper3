#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

static const int id_sequence[] = {7, 13, 21, 34, 55, 89, 144, 233, 377, 610};
#define ID_SEQUENCE_LEN ((size_t)(sizeof(id_sequence)/sizeof(id_sequence[0])))

int get_value_at_index(size_t index) {
    if (index >= ID_SEQUENCE_LEN) {
        return 0; /* invalid index; caller should validate */
    }
    return id_sequence[index];
}

static int parse_index(const char *s, size_t *out_index) {
    if (s == NULL || out_index == NULL) return 0;
    errno = 0;
    char *endptr = NULL;
    long val = strtol(s, &endptr, 10);
    if (errno != 0) return 0;
    while (endptr && *endptr != '\0' && *endptr != '\n') {
        if (!isspace((unsigned char)*endptr)) return 0;
        endptr++;
    }
    if (val < 0) return 0;
    if ((unsigned long)val >= ID_SEQUENCE_LEN) return 0;
    *out_index = (size_t)val;
    return 1;
}

int main(void) {
    printf("Enter an index (0..%zu): ", ID_SEQUENCE_LEN - 1);
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        fprintf(stderr, "Error: no input.\n");
    } else {
        /* If input too long (no newline), treat as invalid to avoid overflow */
        if (strchr(buf, '\n') == NULL && strlen(buf) == sizeof(buf) - 1) {
            fprintf(stderr, "Error: input too long.\n");
        } else {
            size_t idx = 0;
            if (!parse_index(buf, &idx)) {
                fprintf(stderr, "Error: invalid input.\n");
            } else {
                int value = get_value_at_index(idx);
                printf("Value at index %zu = %d\n", idx, value);
            }
        }
    }

    /* 5 test cases */
    size_t tests[5] = {0, ID_SEQUENCE_LEN - 1, 2, 5, 7};
    for (size_t i = 0; i < 5; i++) {
        size_t t = tests[i];
        int v = get_value_at_index(t);
        printf("Test index %zu -> %d\n", t, v);
    }
    return 0;
}