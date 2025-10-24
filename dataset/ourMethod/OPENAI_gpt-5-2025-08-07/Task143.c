#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>

static const size_t MAX_LEN = 1000000;

struct IntArray {
    int *data;
    size_t len;
};

static int cmp_ints(const void *a, const void *b) {
    const int ia = *(const int *)a;
    const int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

struct IntArray sort_ascending(const int *input, size_t len) {
    struct IntArray out;
    out.data = NULL;
    out.len = 0;

    if ((input == NULL && len != 0) || len > MAX_LEN) {
        return out; /* invalid input, fail closed */
    }
    if (len == 0) {
        return out; /* empty result */
    }
    if (len > SIZE_MAX / sizeof(int)) {
        return out; /* overflow guard */
    }

    int *copy = (int *)malloc(len * sizeof(int));
    if (copy == NULL) {
        return out; /* allocation failure */
    }
    memcpy(copy, input, len * sizeof(int));
    qsort(copy, len, sizeof(int), cmp_ints);

    out.data = copy;
    out.len = len;
    return out;
}

static void print_array(const int *arr, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; i++) {
        printf("%d", arr[i]);
        if (i + 1 < len) {
            printf(", ");
        }
    }
    printf("]");
}

static void run_test(int test_num, const int *input, size_t len) {
    struct IntArray sorted = sort_ascending(input, len);
    printf("Test %d input:  ", test_num);
    print_array(input, len);
    printf("\n");
    if (sorted.data == NULL && sorted.len == 0 && len != 0) {
        printf("Test %d error: invalid input or allocation failure\n", test_num);
    } else {
        printf("Test %d output: ", test_num);
        print_array(sorted.data, sorted.len);
        printf("\n");
    }
    printf("----\n");
    if (sorted.data != NULL) {
        free(sorted.data);
    }
}

int main(void) {
    int t1[] = {3, 1, 2};
    int t2[] = {-5, -10, 0, 5};
    int t3[] = {};
    int t4[] = {1, 1, 1, 1};
    int t5[] = {INT_MAX, INT_MIN, 0, -1, 1};

    run_test(1, t1, sizeof(t1) / sizeof(t1[0]));
    run_test(2, t2, sizeof(t2) / sizeof(t2[0]));
    run_test(3, t3, sizeof(t3) / sizeof(t3[0]));
    run_test(4, t4, sizeof(t4) / sizeof(t4[0]));
    run_test(5, t5, sizeof(t5) / sizeof(t5[0]));

    return 0;
}