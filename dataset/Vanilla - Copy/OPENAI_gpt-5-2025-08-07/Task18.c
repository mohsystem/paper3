#include <stdio.h>
#include <stddef.h>

int binaryArrayToNumber(const int* arr, size_t n) {
    int res = 0;
    for (size_t i = 0; i < n; ++i) {
        res = (res << 1) | (arr[i] & 1);
    }
    return res;
}

void run_test(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("] => %d\n", binaryArrayToNumber(arr, n));
}

int main() {
    int a1[] = {0, 0, 0, 1};
    int a2[] = {0, 0, 1, 0};
    int a3[] = {0, 1, 0, 1};
    int a4[] = {1, 0, 0, 1};
    int a5[] = {1, 1, 1, 1};

    run_test(a1, 4);
    run_test(a2, 4);
    run_test(a3, 4);
    run_test(a4, 4);
    run_test(a5, 4);
    return 0;
}