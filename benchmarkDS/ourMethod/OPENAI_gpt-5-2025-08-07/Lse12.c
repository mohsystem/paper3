#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int getValueFromArray(const int *arr, size_t size, long long index) {
    // Returns the value at index if valid; otherwise returns 1 (error code).
    if (arr == NULL) return 1;
    if (index < 0) return 1;
    if ((size_t)index >= size) return 1;
    return arr[(size_t)index];
}

static void run_tests(void) {
    int data[5] = {10, 20, 30, 40, 50};
    assert(getValueFromArray(data, 5, 0) == 10);
    assert(getValueFromArray(data, 5, 1) == 20);
    assert(getValueFromArray(data, 5, 2) == 30);
    assert(getValueFromArray(data, 5, 4) == 50);
    assert(getValueFromArray(data, 5, 5) == 1); // out-of-bounds -> 1
}

int main(void) {
    run_tests();

    int data[5] = {10, 20, 30, 40, 50};

    char buf[128];
    long long idx = -1; // invalid by default
    if (fgets(buf, (int)sizeof(buf), stdin) != NULL) {
        // strip trailing newline
        size_t len = strnlen(buf, sizeof(buf));
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }

        char *endptr = NULL;
        errno = 0;
        long long val = strtoll(buf, &endptr, 10);
        if (errno == 0 && endptr != buf) {
            // skip trailing spaces
            while (*endptr == ' ' || *endptr == '\t' || *endptr == '\r') {
                endptr++;
            }
            if (*endptr == '\0') {
                idx = val;
            }
        }
    }

    int result = getValueFromArray(data, 5, idx);
    return result & 0xFF;
}