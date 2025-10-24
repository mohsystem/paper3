#include <stdio.h>
#include <stddef.h>

int findOdd(const int* arr, size_t len) {
    if (arr == NULL || len == 0) {
        return 0; /* Invalid input; returning 0 as a safe default */
    }
    int x = 0;
    for (size_t i = 0; i < len; ++i) {
        x ^= arr[i];
    }
    return x;
}

int main(void) {
    int test1[] = {7};
    int test2[] = {0};
    int test3[] = {1, 1, 2};
    int test4[] = {0, 1, 0, 1, 0};
    int test5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};

    printf("Test 1 result: %d\n", findOdd(test1, sizeof(test1)/sizeof(test1[0])));
    printf("Test 2 result: %d\n", findOdd(test2, sizeof(test2)/sizeof(test2[0])));
    printf("Test 3 result: %d\n", findOdd(test3, sizeof(test3)/sizeof(test3[0])));
    printf("Test 4 result: %d\n", findOdd(test4, sizeof(test4)/sizeof(test4[0])));
    printf("Test 5 result: %d\n", findOdd(test5, sizeof(test5)/sizeof(test5[0])));

    return 0;
}