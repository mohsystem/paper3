#include <stdio.h>
#include <stddef.h>

const char* odd_or_even(const long long* arr, size_t n) {
    unsigned int parity = 0U;

    if (arr == NULL || n == 0) {
        return "even";
    }

    for (size_t i = 0; i < n; ++i) {
        parity ^= ((unsigned long long)arr[i]) & 1ULL;
    }
    return parity ? "odd" : "even";
}

int main(void) {
    // 5 test cases
    const char* res;

    // Test 1: empty (treated as [0])
    res = odd_or_even(NULL, 0);
    printf("Test 1: %s\n", res);

    // Test 2: [0]
    long long a2[] = {0};
    res = odd_or_even(a2, sizeof(a2)/sizeof(a2[0]));
    printf("Test 2: %s\n", res);

    // Test 3: [0, 1, 4]
    long long a3[] = {0, 1, 4};
    res = odd_or_even(a3, sizeof(a3)/sizeof(a3[0]));
    printf("Test 3: %s\n", res);

    // Test 4: [0, -1, -5]
    long long a4[] = {0, -1, -5};
    res = odd_or_even(a4, sizeof(a4)/sizeof(a4[0]));
    printf("Test 4: %s\n", res);

    // Test 5: [2, 4, 6, 8, 10]
    long long a5[] = {2, 4, 6, 8, 10};
    res = odd_or_even(a5, sizeof(a5)/sizeof(a5[0]));
    printf("Test 5: %s\n", res);

    return 0;
}