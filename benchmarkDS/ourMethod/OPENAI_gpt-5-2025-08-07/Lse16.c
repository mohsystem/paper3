#include <stdio.h>

#define DATA_LEN 10

static const int DATA[DATA_LEN] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int get_value_at_index(int index) {
    if (index < 0 || index >= DATA_LEN) {
        fprintf(stderr, "Invalid index\n");
        return 0;
    }
    return DATA[index];
}

int main(void) {
    int tests[5] = {0, 5, 9, 3, 1};
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        int idx = tests[i];
        int result = get_value_at_index(idx);
        printf("Index %d -> %d\n", idx, result);
    }
    return 0;
}