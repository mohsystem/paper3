#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>

int* removeSmallest(const int* arr, size_t len, size_t* out_len) {
    if (out_len == NULL) return NULL;
    if (arr == NULL || len == 0) {
        *out_len = 0;
        return NULL;
    }
    int minVal = arr[0];
    size_t minIdx = 0;
    for (size_t i = 1; i < len; ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIdx = i;
        }
    }
    *out_len = len - 1;
    if (*out_len == 0) {
        return NULL;
    }
    int* res = (int*)malloc((*out_len) * sizeof(int));
    if (!res) {
        *out_len = 0;
        return NULL;
    }
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        if (i != minIdx) {
            res[j++] = arr[i];
        }
    }
    return res;
}

static void printArray(const int* arr, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; ++i) {
        printf("%d", arr[i]);
        if (i + 1 < len) printf(",");
    }
    printf("]\n");
}

int main(void) {
    int t1[] = {1,2,3,4,5};
    int t2[] = {5,3,2,1,4};
    int t3[] = {2,2,1,2,1};
    int* t4 = NULL; size_t t4_len = 0;
    int t5[] = {7,7,7};

    struct {
        const int* arr;
        size_t len;
    } tests[5] = {
        {t1, sizeof(t1)/sizeof(t1[0])},
        {t2, sizeof(t2)/sizeof(t2[0])},
        {t3, sizeof(t3)/sizeof(t3[0])},
        {t4, t4_len},
        {t5, sizeof(t5)/sizeof(t5[0])}
    };

    for (int i = 0; i < 5; ++i) {
        size_t out_len = 0;
        int* out = removeSmallest(tests[i].arr, tests[i].len, &out_len);
        if (out_len == 0 || out == NULL) {
            printf("[]\n");
        } else {
            printArray(out, out_len);
        }
        free(out);
    }
    return 0;
}