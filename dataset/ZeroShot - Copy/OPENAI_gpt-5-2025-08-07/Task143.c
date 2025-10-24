#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int int_cmp(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

int* sort_ascending(const int* arr, size_t n, size_t* out_n) {
    if (out_n == NULL) {
        return NULL;
    }
    *out_n = 0;
    if (arr == NULL || n == 0) {
        return NULL;
    }
    if (n > SIZE_MAX / sizeof(int)) {
        return NULL;
    }
    int* copy = (int*)malloc(n * sizeof(int));
    if (!copy) {
        return NULL;
    }
    memcpy(copy, arr, n * sizeof(int));
    qsort(copy, n, sizeof(int), int_cmp);
    *out_n = n;
    return copy;
}

static void print_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test case 1
    int t1[] = {5, 2, 9, 1, 5, 6};
    size_t n1 = 0;
    int* r1 = sort_ascending(t1, sizeof(t1)/sizeof(t1[0]), &n1);
    print_array(r1 ? r1 : NULL, r1 ? n1 : 0);
    free(r1);

    // Test case 2
    int t2[] = {-3, 0, -1, -7, 2};
    size_t n2 = 0;
    int* r2 = sort_ascending(t2, sizeof(t2)/sizeof(t2[0]), &n2);
    print_array(r2 ? r2 : NULL, r2 ? n2 : 0);
    free(r2);

    // Test case 3
    int* t3 = NULL;
    size_t n3 = 0;
    int* r3 = sort_ascending(t3, 0, &n3);
    print_array(r3 ? r3 : NULL, r3 ? n3 : 0);
    free(r3);

    // Test case 4
    int t4[] = {42};
    size_t n4 = 0;
    int* r4 = sort_ascending(t4, sizeof(t4)/sizeof(t4[0]), &n4);
    print_array(r4 ? r4 : NULL, r4 ? n4 : 0);
    free(r4);

    // Test case 5
    int t5[] = {INT_MAX, INT_MIN, 0, -1, 1};
    size_t n5 = 0;
    int* r5 = sort_ascending(t5, sizeof(t5)/sizeof(t5[0]), &n5);
    print_array(r5 ? r5 : NULL, r5 ? n5 : 0);
    free(r5);

    return 0;
}