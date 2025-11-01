#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

int getValueFromArray(const int *arr, size_t size, int index) {
    if (arr == NULL || size == 0) return 0;
    if (index < 0) return 0;
    size_t uindex = (size_t)index;
    if (uindex >= size) return 0;
    return arr[uindex];
}

int main(void) {
    const int id_sequence[] = {7, 13, 42, -1, 255, 1024, -300};
    size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    // 5 test cases
    int testIndices[5] = {0, 3, 6, -1, 100};
    for (int i = 0; i < 5; ++i) {
        int val = getValueFromArray(id_sequence, size, testIndices[i]);
        printf("Test %d index=%d value=%d\n", i + 1, testIndices[i], val);
    }

    int idx = 0;
    char buf[128];
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        errno = 0;
        char *endptr = NULL;
        long v = strtol(buf, &endptr, 10);
        if (errno == 0) {
            if (v > INT_MAX) idx = INT_MAX;
            else if (v < INT_MIN) idx = INT_MIN;
            else idx = (int)v;
        } else {
            idx = 0;
        }
    } else {
        idx = 0;
    }

    int value = getValueFromArray(id_sequence, size, idx);
    int exitCode = value & 0xFF;
    return exitCode;
}