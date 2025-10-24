#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int *data;
    size_t len;
} IntArray;

static void freeIntArray(IntArray *a) {
    if (a != NULL && a->data != NULL) {
        free(a->data);
        a->data = NULL;
    }
    if (a != NULL) {
        a->len = 0;
    }
}

static void printIntArray(const IntArray *a) {
    if (a == NULL) {
        printf("[]");
        return;
    }
    printf("[");
    for (size_t i = 0; i < a->len; ++i) {
        if (i) printf(",");
        printf("%d", a->data[i]);
    }
    printf("]");
}

IntArray removeSmallest(const int *arr, size_t len) {
    IntArray out = { NULL, 0 };

    // Validate inputs: if len > 0, arr must be non-NULL
    if (len == 0 || arr == NULL) {
        // Empty output on invalid/empty input
        return out;
    }

    // Find index of first smallest value
    size_t minIndex = 0;
    int minVal = arr[0];
    for (size_t i = 1; i < len; ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIndex = i;
        }
    }

    // Handle case where result length is zero
    if (len == 1) {
        // Removing the only element yields empty array
        return out;
    }

    size_t newLen = len - 1;
    out.data = (int*)malloc(newLen * sizeof(int));
    if (out.data == NULL) {
        // Allocation failed; return empty
        out.len = 0;
        return out;
    }

    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        if (i == minIndex) continue;
        if (j < newLen) {
            out.data[j++] = arr[i];
        } else {
            // Bounds safety: should not occur, but fail closed
            freeIntArray(&out);
            return (IntArray){NULL, 0};
        }
    }
    out.len = newLen;
    return out;
}

int main(void) {
    // Test cases
    int t1[] = {1,2,3,4,5};
    int t2[] = {5,3,2,1,4};
    int t3[] = {2,2,1,2,1};
    // t4: empty input
    // t5: single element
    int t5[] = {7};

    IntArray tests_in[5];
    tests_in[0] = (IntArray){ t1, sizeof(t1)/sizeof(t1[0]) };
    tests_in[1] = (IntArray){ t2, sizeof(t2)/sizeof(t2[0]) };
    tests_in[2] = (IntArray){ t3, sizeof(t3)/sizeof(t3[0]) };
    tests_in[3] = (IntArray){ NULL, 0 };
    tests_in[4] = (IntArray){ t5, sizeof(t5)/sizeof(t5[0]) };

    for (size_t i = 0; i < 5; ++i) {
        printf("Input: ");
        if (tests_in[i].len == 0) {
            printf("[]");
        } else {
            IntArray tmpIn = { tests_in[i].data, tests_in[i].len };
            printIntArray(&tmpIn);
        }
        IntArray out = removeSmallest(tests_in[i].data, tests_in[i].len);
        printf(" -> Output: ");
        printIntArray(&out);
        printf("\n");
        freeIntArray(&out);
    }

    return 0;
}