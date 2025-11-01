#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/*
 * Returns 0 on success and writes the value to *out_value.
 * Returns -1 on error (invalid parameters or index).
 */
int get_value_at_index(const int *id_sequence, size_t len, size_t index, int *out_value) {
    if (id_sequence == NULL || out_value == NULL) {
        return -1;
    }
    if (index >= len) {
        return -1;
    }
    *out_value = id_sequence[index];
    return 0;
}

/*
 * Prompts the user, reads an index, validates it, and on success
 * writes the value at that index to *out_value and returns 0.
 * Returns -1 on any error or invalid input.
 */
int prompt_and_get_value(const int *id_sequence, size_t len, int *out_value) {
    if (id_sequence == NULL || out_value == NULL) {
        return -1;
    }
    char buf[64];
    printf("Enter index (0..%zu): ", (len == 0 ? (size_t)0 : len - 1));
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        printf("No input.\n");
        return -1;
    }
    size_t l = strnlen(buf, sizeof(buf));
    if (l == 0) {
        printf("Invalid input.\n");
        return -1;
    }
    if (buf[l - 1] == '\n') {
        buf[l - 1] = '\0';
        l--;
    }
    if (l == 0 || l > 18) {
        printf("Invalid input.\n");
        return -1;
    }
    errno = 0;
    char *endptr = NULL;
    long long v = strtoll(buf, &endptr, 10);
    if (errno != 0 || endptr == buf || *endptr != '\0' || v < 0) {
        printf("Invalid input.\n");
        return -1;
    }
    size_t idx = (size_t)v;
    int value = 0;
    if (get_value_at_index(id_sequence, len, idx, &value) != 0) {
        printf("Invalid index.\n");
        return -1;
    }
    *out_value = value;
    return 0;
}

int main(void) {
    int id_sequence[] = {42, 7, 13, 99, 5, 81, 23, 64, 12, 30};
    size_t len = sizeof(id_sequence) / sizeof(id_sequence[0]);

    for (int t = 0; t < 5; ++t) {
        int result = 0;
        if (prompt_and_get_value(id_sequence, len, &result) == 0) {
            printf("Value: %d\n", result);
        } else {
            printf("No value returned.\n");
        }
    }
    return 0;
}