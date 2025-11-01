#include <stdio.h>
#include <stddef.h>

// Immutable ID sequence
static const int ID_SEQUENCE[] = {101, 202, 303, 404, 505};
static const size_t ID_SEQUENCE_LEN = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

// Returns the value at the given index, or -1 if index is invalid
int getIdAtIndex(int index) {
    if (index < 0 || (size_t)index >= ID_SEQUENCE_LEN) {
        return -1; // Sentinel for invalid index
    }
    return ID_SEQUENCE[(size_t)index];
}

int main(void) {
    int tests[5] = {0, 4, 2, -1, 10}; // 5 test cases
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        printf("%d\n", getIdAtIndex(tests[i]));
    }
    return 0;
}