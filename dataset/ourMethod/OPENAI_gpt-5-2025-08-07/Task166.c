#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int* data;
    size_t size;
} Array;

static int cmp_int_asc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

Array posNegSort(const int* arr, size_t n) {
    Array res;
    res.data = NULL;
    res.size = 0;

    if (arr == NULL || n == 0) {
        return res;
    }

    size_t pos_count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (arr[i] > 0) pos_count++;
    }

    int* positives = NULL;
    if (pos_count > 0) {
        positives = (int*)malloc(pos_count * sizeof(int));
        if (positives == NULL) {
            return res; /* allocation failed, fail closed with empty result */
        }
    }

    size_t p = 0;
    for (size_t i = 0; i < n; ++i) {
        if (arr[i] > 0) positives[p++] = arr[i];
    }

    if (pos_count > 0) {
        qsort(positives, pos_count, sizeof(int), cmp_int_asc);
    }

    int* out = (int*)malloc(n * sizeof(int));
    if (out == NULL) {
        if (positives) free(positives);
        return res; /* allocation failed */
    }

    size_t pi = 0;
    for (size_t i = 0; i < n; ++i) {
        if (arr[i] < 0) {
            out[i] = arr[i];
        } else {
            out[i] = positives[pi++];
        }
    }

    if (positives) free(positives);
    res.data = out;
    res.size = n;
    return res;
}

static void printArray(const int* a, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", a[i]);
    }
    printf("]");
}

static void freeArray(Array* a) {
    if (a && a->data) {
        free(a->data);
        a->data = NULL;
        a->size = 0;
    }
}

int main(void) {
    int t1[] = {6, 3, -2, 5, -8, 2, -2};
    int t2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int t3[] = {-5, -5, -5, -5, 7, -5};
    int t4[] = {};
    int t5[] = {-1, -3, 2, 1, 4};

    int* tests[] = {t1, t2, t3, t4, t5};
    size_t sizes[] = {
        sizeof(t1)/sizeof(t1[0]),
        sizeof(t2)/sizeof(t2[0]),
        sizeof(t3)/sizeof(t3[0]),
        sizeof(t4)/sizeof(t4[0]),
        sizeof(t5)/sizeof(t5[0])
    };

    for (size_t i = 0; i < 5; ++i) {
        printf("Test %zu input:  ", i + 1);
        printArray(tests[i], sizes[i]);
        Array out = posNegSort(tests[i], sizes[i]);
        printf("\nTest %zu output: ", i + 1);
        printArray(out.data ? out.data : (int[]){0}, out.size);
        printf("\n\n");
        freeArray(&out);
    }

    return 0;
}