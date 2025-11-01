#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int getValueFromArray(const int* arr, size_t size, int index) {
    if (arr == NULL || size == 0) {
        return 0;
    }
    if (index >= 0 && (size_t)index < size) {
        return arr[index];
    }
    return 0;
}

void runTests() {
    int arr[5] = {10, 20, 30, 40, 50};
    size_t size = 5;
    int tests[5] = {0, 2, 4, -1, 10};
    for (int i = 0; i < 5; ++i) {
        int res = getValueFromArray(arr, size, tests[i]);
        printf("Test %d: index=%d value=%d\n", i + 1, tests[i], res);
    }
}

int main(void) {
    runTests();

    int arr[5] = {10, 20, 30, 40, 50};
    size_t size = 5;
    int index = 0;

    char buf[128];
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        char *endptr = NULL;
        errno = 0;
        long val = strtol(buf, &endptr, 10);
        if (errno == 0 && endptr != buf) {
            index = (int)val;
        } else {
            index = 0;
        }
    } else {
        index = 0;
    }

    int value = getValueFromArray(arr, size, index);
    return value;
}