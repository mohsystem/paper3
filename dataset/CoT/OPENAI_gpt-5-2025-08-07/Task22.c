#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TYPE_INT = 0,
    TYPE_STR = 1
} ValueType;

typedef struct {
    ValueType type;
    int int_val;
    const char* str_val;
} Item;

int filter_list(const Item* items, size_t n, int** out_arr, size_t* out_len) {
    if (!out_arr || !out_len) return -1;
    *out_arr = NULL;
    *out_len = 0;
    if (!items || n == 0) return 0;

    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (items[i].type == TYPE_INT && items[i].int_val >= 0) {
            ++count;
        }
    }

    if (count == 0) return 0;

    int* arr = (int*)malloc(count * sizeof(int));
    if (!arr) return -2;

    size_t idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (items[i].type == TYPE_INT && items[i].int_val >= 0) {
            arr[idx++] = items[i].int_val;
        }
    }

    *out_arr = arr;
    *out_len = idx;
    return 0;
}

static void print_int_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]");
}

int main(void) {
    Item t1[] = { {TYPE_INT, 1, NULL}, {TYPE_INT, 2, NULL}, {TYPE_STR, 0, "a"}, {TYPE_STR, 0, "b"} };
    Item t2[] = { {TYPE_INT, 1, NULL}, {TYPE_STR, 0, "a"}, {TYPE_STR, 0, "b"}, {TYPE_INT, 0, NULL}, {TYPE_INT, 15, NULL} };
    Item t3[] = { {TYPE_INT, 1, NULL}, {TYPE_INT, 2, NULL}, {TYPE_STR, 0, "a"}, {TYPE_STR, 0, "b"}, {TYPE_STR, 0, "aasf"}, {TYPE_STR, 0, "1"}, {TYPE_STR, 0, "123"}, {TYPE_INT, 123, NULL} };
    Item t4[] = { {TYPE_INT, 0, NULL}, {TYPE_STR, 0, "zero"}, {TYPE_INT, 99, NULL}, {TYPE_STR, 0, "99"}, {TYPE_INT, 100, NULL}, {TYPE_INT, -5, NULL} };
    Item t5[] = { {TYPE_STR, 0, "only"}, {TYPE_STR, 0, "strings"}, {TYPE_STR, 0, "here"} };

    Item* tests[] = { t1, t2, t3, t4, t5 };
    size_t sizes[] = { sizeof(t1)/sizeof(t1[0]), sizeof(t2)/sizeof(t2[0]), sizeof(t3)/sizeof(t3[0]), sizeof(t4)/sizeof(t4[0]), sizeof(t5)/sizeof(t5[0]) };

    for (size_t i = 0; i < 5; ++i) {
        int* out = NULL;
        size_t out_len = 0;
        int rc = filter_list(tests[i], sizes[i], &out, &out_len);
        if (rc == 0) {
            printf("Test %zu: ", i + 1);
            print_int_array(out, out_len);
            printf("\n");
        } else {
            printf("Test %zu: error %d\n", i + 1, rc);
        }
        free(out);
    }

    return 0;
}