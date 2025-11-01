#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int getValueFromArray(const int *arr, size_t size, int index) {
    if (arr == NULL) {
        errno = EINVAL;
        return 0;
    }
    if (size > (size_t)1000000000u) { /* arbitrary large sanity check */
        errno = EINVAL;
        return 0;
    }
    if (index < 0 || (size_t)index >= size) {
        errno = EINVAL;
        return 0;
    }
    return arr[(size_t)index];
}

int main(void) {
    const int id_sequence[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    /* 5 test cases */
    const int testIndices[5] = {0, 3, 9, 5, 7};
    for (size_t i = 0; i < 5; ++i) {
        int idx = testIndices[i];
        if (idx >= 0 && (size_t)idx < size) {
            int val = getValueFromArray(id_sequence, size, idx);
            printf("Test index %d => %d\n", idx, val);
        } else {
            printf("Test index %d => ERROR\n", idx);
        }
    }

    /* Read one integer index from stdin and return value as exit code */
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        fprintf(stderr, "ERROR\n");
        return 1;
    }

    char *endptr = NULL;
    errno = 0;
    long idxl = strtol(buf, &endptr, 10);
    if ((errno == ERANGE) || endptr == buf) {
        fprintf(stderr, "ERROR\n");
        return 1;
    }
    /* Ensure remaining characters are only whitespace/newline */
    while (*endptr != '\0') {
        if (*endptr != ' ' && *endptr != '\t' && *endptr != '\r' && *endptr != '\n') {
            fprintf(stderr, "ERROR\n");
            return 1;
        }
        endptr++;
    }
    if (idxl < 0 || (unsigned long)idxl >= size) {
        fprintf(stderr, "ERROR\n");
        return 1;
    }

    int index = (int)idxl;
    int result = getValueFromArray(id_sequence, size, index);
    /* If getValueFromArray failed due to internal validation, treat as error */
    if (errno == EINVAL && (index < 0 || (size_t)index >= size)) {
        fprintf(stderr, "ERROR\n");
        return 1;
    }
    return result;
}