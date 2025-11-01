#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

int getValueFromArray(const int* arr, size_t size, int index) {
    if (arr == NULL) {
        return 0;
    }
    if (index < 0) {
        return 0;
    }
    if ((size_t)index >= size) {
        return 0;
    }
    return arr[index];
}

int main(void) {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 127, 255, 300, -5};
    size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    /* 5 test cases (not affecting exit code) */
    int testIndices[5];
    testIndices[0] = -1;
    testIndices[1] = 0;
    testIndices[2] = 3;
    testIndices[3] = (int)size - 1;
    testIndices[4] = 999;
    volatile int dummy = 0;
    for (int i = 0; i < 5; ++i) {
        dummy ^= getValueFromArray(id_sequence, size, testIndices[i]);
    }
    if (dummy == INT_MIN) {
        (void)dummy;
    }

    char buf[256];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        buf[0] = '\0';
    }

    char *endptr = NULL;
    long idxLong = strtol(buf, &endptr, 10);
    if (endptr == buf) {
        idxLong = 0;
    }
    int inputIndex;
    if (idxLong > INT_MAX) inputIndex = INT_MAX;
    else if (idxLong < INT_MIN) inputIndex = INT_MIN;
    else inputIndex = (int)idxLong;

    int result = getValueFromArray(id_sequence, size, inputIndex);
    int exitCode = result & 0xFF;
    return exitCode;
}