#include <stdio.h>

static const int DATA[] = {10, 20, 30, 40, 50, 60, 70, 80};
#define DATA_LEN (sizeof(DATA) / sizeof(DATA[0]))

int get_value_at_index(int index, int* out_value) {
    if (out_value == NULL) {
        return -2; // invalid output pointer
    }
    if (index < 0 || index >= (int)DATA_LEN) {
        return -1; // index out of range
    }
    *out_value = DATA[index];
    return 0; // success
}

int main(void) {
    int tests[5] = {0, 1, 2, 5, 7};
    for (size_t i = 0; i < 5; i++) {
        int value = 0;
        int rc = get_value_at_index(tests[i], &value);
        if (rc == 0) {
            printf("Index %d -> %d\n", tests[i], value);
        } else {
            printf("Index %d -> error: %d\n", tests[i], rc);
        }
    }
    return 0;
}