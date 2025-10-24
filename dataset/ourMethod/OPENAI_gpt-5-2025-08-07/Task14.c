#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

int find_even_index(const long long* arr, size_t len) {
    if ((arr == NULL && len > 0) || len == 0) {
        return -1;
    }
    long long total = 0;
    for (size_t i = 0; i < len; ++i) {
        total += arr[i];
    }
    long long left = 0;
    for (size_t i = 0; i < len; ++i) {
        long long right = total - left - arr[i];
        if (left == right) {
            return (int)i;
        }
        left += arr[i];
    }
    return -1;
}

int main(void) {
    // 5 test cases
    long long a1[] = {1, 2, 3, 4, 3, 2, 1};           // Expected 3
    long long a2[] = {1, 100, 50, -51, 1, 1};         // Expected 1
    long long a3[] = {20, 10, -80, 10, 10, 15, 35};   // Expected 0
    long long a4[] = {1, 2, 3, 4, 5, 6};              // Expected -1
    long long a5[] = {0, 0, 0, 0};                    // Expected 0

    int r1 = find_even_index(a1, sizeof(a1) / sizeof(a1[0]));
    int r2 = find_even_index(a2, sizeof(a2) / sizeof(a2[0]));
    int r3 = find_even_index(a3, sizeof(a3) / sizeof(a3[0]));
    int r4 = find_even_index(a4, sizeof(a4) / sizeof(a4[0]));
    int r5 = find_even_index(a5, sizeof(a5) / sizeof(a5[0]));

    printf("Test 1 result: %d\n", r1);
    printf("Test 2 result: %d\n", r2);
    printf("Test 3 result: %d\n", r3);
    printf("Test 4 result: %d\n", r4);
    printf("Test 5 result: %d\n", r5);

    return 0;
}