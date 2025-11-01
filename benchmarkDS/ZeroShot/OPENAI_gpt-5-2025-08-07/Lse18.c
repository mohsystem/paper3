#include <stdio.h>
#include <stddef.h>

typedef struct {
    int success;
    int value;
} Result;

Result getValueAtIndex(int index) {
    static const int DATA[] = {11, 22, 33, 44, 55, 66, 77, 88, 99, 110};
    const size_t len = sizeof(DATA) / sizeof(DATA[0]);
    Result r;
    if (index < 0 || (size_t)index >= len) {
        r.success = 0;
        r.value = 0;
        return r;
    }
    r.success = 1;
    r.value = DATA[(size_t)index];
    return r;
}

int main(void) {
    int tests[5] = {0, 5, 9, 3, 7};
    for (size_t i = 0; i < 5; ++i) {
        int t = tests[i];
        Result r = getValueAtIndex(t);
        if (r.success) {
            printf("Index %d -> %d\n", t, r.value);
        } else {
            printf("Index %d -> error: index out of bounds\n", t);
        }
    }
    return 0;
}