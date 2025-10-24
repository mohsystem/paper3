#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int is_int;        /* 1 if int, 0 if string */
    int int_value;
    const char* str_value;
} Item;

typedef struct {
    int* data;
    size_t size;
} IntArray;

static int safe_mul_size(size_t a, size_t b, size_t* out) {
    if (a == 0 || b == 0) { *out = 0; return 1; }
    if (a > SIZE_MAX / b) return 0;
    *out = a * b;
    return 1;
}

IntArray filter_list(const Item* items, size_t n) {
    IntArray res = {NULL, 0};
    if (items == NULL || n == 0) {
        return res;
    }

    size_t bytes = 0;
    if (!safe_mul_size(n, sizeof(int), &bytes)) {
        return res; /* overflow -> return empty */
    }

    int* buf = NULL;
    if (bytes == 0) {
        buf = NULL;
    } else {
        buf = (int*)calloc(n, sizeof(int));
        if (buf == NULL) {
            return res; /* allocation failed */
        }
    }

    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (items[i].is_int) {
            int v = items[i].int_value;
            if (v >= 0) {
                buf[count++] = v;
            }
        }
    }

    if (count == 0) {
        free(buf);
        res.data = NULL;
        res.size = 0;
        return res;
    }

    /* shrink to fit */
    int* shrunk = (int*)realloc(buf, count * sizeof(int));
    if (shrunk == NULL) {
        /* if realloc fails, keep original buffer */
        res.data = buf;
        res.size = count;
    } else {
        res.data = shrunk;
        res.size = count;
    }
    return res;
}

static void print_int_array(const IntArray* arr) {
    printf("[");
    if (arr != NULL && arr->data != NULL) {
        for (size_t i = 0; i < arr->size; ++i) {
            if (i) printf(", ");
            printf("%d", arr->data[i]);
        }
    }
    printf("]\n");
}

static void free_int_array(IntArray* arr) {
    if (arr) {
        free(arr->data);
        arr->data = NULL;
        arr->size = 0;
    }
}

int main(void) {
    /* Test case 1 */
    Item t1[] = {
        {1, 1, NULL}, {1, 2, NULL}, {0, 0, "a"}, {0, 0, "b"}
    };
    IntArray r1 = filter_list(t1, sizeof(t1)/sizeof(t1[0]));
    print_int_array(&r1); /* [1, 2] */
    free_int_array(&r1);

    /* Test case 2 */
    Item t2[] = {
        {1, 1, NULL}, {0, 0, "a"}, {0, 0, "b"}, {1, 0, NULL}, {1, 15, NULL}
    };
    IntArray r2 = filter_list(t2, sizeof(t2)/sizeof(t2[0]));
    print_int_array(&r2); /* [1, 0, 15] */
    free_int_array(&r2);

    /* Test case 3 */
    Item t3[] = {
        {1, 1, NULL}, {1, 2, NULL}, {0, 0, "a"}, {0, 0, "b"},
        {0, 0, "aasf"}, {0, 0, "1"}, {0, 0, "123"}, {1, 123, NULL}
    };
    IntArray r3 = filter_list(t3, sizeof(t3)/sizeof(t3[0]));
    print_int_array(&r3); /* [1, 2, 123] */
    free_int_array(&r3);

    /* Test case 4 (includes negative integer) */
    Item t4[] = {
        {0, 0, "hello"}, {1, -1, NULL}, {1, 3, NULL}, {0, 0, "4"}, {1, 0, NULL}
    };
    IntArray r4 = filter_list(t4, sizeof(t4)/sizeof(t4[0]));
    print_int_array(&r4); /* [3, 0] */
    free_int_array(&r4);

    /* Test case 5 (empty list) */
    Item* t5 = NULL;
    IntArray r5 = filter_list(t5, 0);
    print_int_array(&r5); /* [] */
    free_int_array(&r5);

    return 0;
}