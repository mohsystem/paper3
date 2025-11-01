#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/*
 * Returns 0 on success and writes the value to *out_value.
 * Returns -1 on error (invalid index or null out_value).
 */
int get_id_at_index(int index, int *out_value) {
    static const int ID_SEQUENCE[] = {101, 203, 304, 405, 506, 607, 708, 809, 910, 1001};
    const size_t LEN = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

    if (out_value == NULL) {
        return -1;
    }
    if (index < 0 || (size_t)index >= LEN) {
        return -1;
    }
    *out_value = ID_SEQUENCE[(size_t)index];
    return 0;
}

int main(void) {
    char buf[64];
    printf("Enter an index (0..9): ");
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        char *endptr = NULL;
        errno = 0;
        long val = strtol(buf, &endptr, 10);
        if (errno != 0 || endptr == buf) {
            printf("Invalid input.\n");
        } else {
            while (*endptr == ' ' || *endptr == '\t') {
                endptr++;
            }
            if (*endptr != '\n' && *endptr != '\0') {
                printf("Invalid input.\n");
            } else if (val < INT_MIN || val > INT_MAX) {
                printf("Invalid input.\n");
            } else {
                int idx = (int)val;
                int result = 0;
                if (get_id_at_index(idx, &result) == 0) {
                    printf("Value at index %d: %d\n", idx, result);
                } else {
                    printf("Invalid index.\n");
                }
            }
        }
    } else {
        printf("I/O error.\n");
    }

    // 5 Test cases
    printf("Running test cases:\n");
    int tests[5] = {0, 3, 9, -1, 10};
    for (size_t i = 0; i < 5; i++) {
        int value = 0;
        if (get_id_at_index(tests[i], &value) == 0) {
            printf("Index %d => %d\n", tests[i], value);
        } else {
            printf("Index %d => error\n", tests[i]);
        }
    }
    return 0;
}