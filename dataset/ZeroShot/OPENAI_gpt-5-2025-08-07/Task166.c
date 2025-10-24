#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_int_asc(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

int* posNegSort(const int* arr, size_t len) {
    if (arr == NULL) return NULL;
    int* res = (int*)malloc(len * sizeof(int));
    if (!res) return NULL;
    memcpy(res, arr, len * sizeof(int));

    size_t count_pos = 0;
    for (size_t i = 0; i < len; ++i) {
        if (arr[i] > 0) count_pos++;
    }
    int* pos = NULL;
    if (count_pos > 0) {
        pos = (int*)malloc(count_pos * sizeof(int));
        if (!pos) {
            free(res);
            return NULL;
        }
        size_t idx = 0;
        for (size_t i = 0; i < len; ++i) {
            if (arr[i] > 0) pos[idx++] = arr[i];
        }
        qsort(pos, count_pos, sizeof(int), cmp_int_asc);

        size_t pi = 0;
        for (size_t i = 0; i < len; ++i) {
            if (res[i] > 0) {
                if (pi < count_pos) res[i] = pos[pi++];
            }
        }
        free(pos);
    }
    return res;
}

static void print_array(const int* arr, size_t len) {
    if (!arr) {
        printf("null\n");
        return;
    }
    printf("[");
    for (size_t i = 0; i < len; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    int t1[] = {6, 3, -2, 5, -8, 2, -2};
    int t2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int t3[] = {-5, -5, -5, -5, 7, -5};
    int t4[] = {};
    int t5[] = {5, -1, -3, 2, 1};

    struct { int* data; size_t len; } tests[] = {
        { t1, sizeof(t1)/sizeof(t1[0]) },
        { t2, sizeof(t2)/sizeof(t2[0]) },
        { t3, sizeof(t3)/sizeof(t3[0]) },
        { t4, sizeof(t4)/sizeof(t4[0]) },
        { t5, sizeof(t5)/sizeof(t5[0]) }
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        int* out = posNegSort(tests[i].data, tests[i].len);
        print_array(out, tests[i].len);
        free(out);
    }
    return 0;
}