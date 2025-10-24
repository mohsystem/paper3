#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

int* merge_sorted(const int* a, size_t lenA, const int* b, size_t lenB, size_t* outLen) {
    if (outLen == NULL) {
        return NULL;
    }
    *outLen = 0;

    if ((a == NULL && lenA != 0) || (b == NULL && lenB != 0)) {
        return NULL;
    }

    if (lenA > SIZE_MAX - lenB) {
        return NULL; // overflow in addition
    }
    size_t total = lenA + lenB;

    if (total > SIZE_MAX / sizeof(int)) {
        return NULL; // overflow in multiplication
    }

    int* result = (int*)malloc(total * sizeof(int));
    if (result == NULL) {
        return NULL;
    }

    size_t i = 0, j = 0, k = 0;
    while (i < lenA && j < lenB) {
        if (a[i] <= b[j]) {
            result[k++] = a[i++];
        } else {
            result[k++] = b[j++];
        }
    }
    while (i < lenA) result[k++] = a[i++];
    while (j < lenB) result[k++] = b[j++];

    *outLen = total;
    return result;
}

void print_array(const int* arr, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test case 1
    size_t out1 = 0;
    int* m1 = merge_sorted(NULL, 0, NULL, 0, &out1);
    if (m1 || out1 == 0) {
        print_array(m1 ? m1 : (int[]){0}, out1);
    }
    free(m1);

    // Test case 2
    int a2[] = {1, 3, 5};
    int b2[] = {2, 4, 6};
    size_t out2 = 0;
    int* m2 = merge_sorted(a2, 3, b2, 3, &out2);
    if (m2) print_array(m2, out2);
    free(m2);

    // Test case 3
    int b3[] = {7, 8};
    size_t out3 = 0;
    int* m3 = merge_sorted(NULL, 0, b3, 2, &out3);
    if (m3) print_array(m3, out3);
    free(m3);

    // Test case 4
    int a4[] = {-5, -1, 0, 2};
    int b4[] = {-4, -3, 3, 10};
    size_t out4 = 0;
    int* m4 = merge_sorted(a4, 4, b4, 4, &out4);
    if (m4) print_array(m4, out4);
    free(m4);

    // Test case 5
    int a5[] = {1, 2, 2, 3};
    int b5[] = {2, 2, 4, 5};
    size_t out5 = 0;
    int* m5 = merge_sorted(a5, 4, b5, 4, &out5);
    if (m5) print_array(m5, out5);
    free(m5);

    return 0;
}