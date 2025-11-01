#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

int getValueFromArray(const int *arr, size_t size, size_t index) {
    if (arr == NULL) {
        return 0;
    }
    if (index >= size) {
        return 0;
    }
    return arr[index];
}

static int to_exit_code(int value) {
    return value & 0xFF;
}

int main(void) {
    const int example[] = {10, 20, 30, 40, 50};
    const size_t size = sizeof(example) / sizeof(example[0]);

    char buf[128];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        // No input provided: run 5 test cases
        for (size_t i = 0; i < size; ++i) {
            int v = getValueFromArray(example, size, i);
            printf("Test %zu: %d\n", i + 1, v);
        }
        return 0;
    }

    // Trim leading/trailing whitespace
    char *start = buf;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') {
        start++;
    }
    char *end = start + strlen(start);
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
        end--;
    }
    *end = '\0';

    if (*start == '\0') {
        // Empty input: run tests
        for (size_t i = 0; i < size; ++i) {
            int v = getValueFromArray(example, size, i);
            printf("Test %zu: %d\n", i + 1, v);
        }
        return 0;
    }

    errno = 0;
    char *ep = NULL;
    long val = strtol(start, &ep, 10);
    if (errno != 0 || ep == start || *ep != '\0') {
        return 1;
    }
    if (val < 0 || (unsigned long)val >= size) {
        return 1;
    }

    int result = getValueFromArray(example, size, (size_t)val);
    return to_exit_code(result);
}