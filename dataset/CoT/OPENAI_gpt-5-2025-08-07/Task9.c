/* Step 1: Problem understanding and definition */
/* Step 2: Security requirements consideration */
/* Step 3: Secure coding generation */
/* Step 4: Code review simulated through comments */
/* Step 5: Secure code output */

#include <stdio.h>
#include <stddef.h>

const char* odd_or_even(const int* arr, size_t len) {
    int parity = 0; /* 0 for even, 1 for odd */
    if (arr != NULL) {
        for (size_t i = 0; i < len; ++i) {
            parity ^= (arr[i] & 1); /* safe for negatives */
        }
    }
    return parity == 0 ? "even" : "odd";
}

int main() {
    /* Test 1: empty array -> even */
    const int* t0 = NULL;
    size_t len0 = 0;

    /* Test 2: [0] -> even */
    int t1[] = {0};
    size_t len1 = sizeof(t1) / sizeof(t1[0]);

    /* Test 3: [0, 1, 4] -> odd */
    int t2[] = {0, 1, 4};
    size_t len2 = sizeof(t2) / sizeof(t2[0]);

    /* Test 4: [0, -1, -5] -> even */
    int t3[] = {0, -1, -5};
    size_t len3 = sizeof(t3) / sizeof(t3[0]);

    /* Test 5: [2, 2, 2, 2] -> even */
    int t4[] = {2, 2, 2, 2};
    size_t len4 = sizeof(t4) / sizeof(t4[0]);

    printf("%s\n", odd_or_even(t0, len0));
    printf("%s\n", odd_or_even(t1, len1));
    printf("%s\n", odd_or_even(t2, len2));
    printf("%s\n", odd_or_even(t3, len3));
    printf("%s\n", odd_or_even(t4, len4));

    return 0;
}