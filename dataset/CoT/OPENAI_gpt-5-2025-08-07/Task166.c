#include <stdio.h>
#include <stdlib.h>

static int cmp_int_asc(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

// Returns a newly allocated array of length n. Caller must free().
int* posNegSort(const int* arr, int n) {
    if (arr == NULL || n <= 0) {
        return NULL;
    }
    int* result = (int*)malloc((size_t)n * sizeof(int));
    if (!result) return NULL;

    int pos_count = 0;
    for (int i = 0; i < n; ++i) {
        if (arr[i] > 0) ++pos_count;
    }

    int* positives = NULL;
    if (pos_count > 0) {
        positives = (int*)malloc((size_t)pos_count * sizeof(int));
        if (!positives) {
            free(result);
            return NULL;
        }
        int idx = 0;
        for (int i = 0; i < n; ++i) {
            if (arr[i] > 0) positives[idx++] = arr[i];
        }
        qsort(positives, (size_t)pos_count, sizeof(int), cmp_int_asc);
    }

    int pidx = 0;
    for (int i = 0; i < n; ++i) {
        if (arr[i] < 0) {
            result[i] = arr[i];
        } else {
            result[i] = positives[pidx++];
        }
    }

    free(positives);
    return result;
}

static void print_array(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    int t1[] = {6, 3, -2, 5, -8, 2, -2};
    int t2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int t3[] = {-5, -5, -5, -5, 7, -5};
    int* t4 = NULL; int n4 = 0;
    int t5[] = {5, -1, -2, 4, 3};

    int* r1 = posNegSort(t1, (int)(sizeof(t1)/sizeof(t1[0])));
    int* r2 = posNegSort(t2, (int)(sizeof(t2)/sizeof(t2[0])));
    int* r3 = posNegSort(t3, (int)(sizeof(t3)/sizeof(t3[0])));
    int* r4 = posNegSort(t4, n4);
    int* r5 = posNegSort(t5, (int)(sizeof(t5)/sizeof(t5[0])));

    print_array(r1, (int)(sizeof(t1)/sizeof(t1[0])));
    print_array(r2, (int)(sizeof(t2)/sizeof(t2[0])));
    print_array(r3, (int)(sizeof(t3)/sizeof(t3[0])));
    print_array(r4, n4);
    print_array(r5, (int)(sizeof(t5)/sizeof(t5[0])));

    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5);

    return 0;
}