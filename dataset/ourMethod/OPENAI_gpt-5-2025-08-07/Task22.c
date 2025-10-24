#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef enum {
    ITEM_INT = 0,
    ITEM_STR = 1
} ItemType;

typedef struct {
    ItemType type;
    long long i;      /* valid when type == ITEM_INT */
    const char* s;    /* valid when type == ITEM_STR */
} Item;

typedef struct {
    long long* data;
    size_t len;
} IntArray;

IntArray filter_list(const Item* arr, size_t len) {
    IntArray result;
    result.data = NULL;
    result.len = 0;

    if ((arr == NULL && len != 0) || (len > 0 && arr == NULL)) {
        return result; /* fail closed */
    }

    if (len == 0) {
        return result;
    }

    if (len > SIZE_MAX / sizeof(long long)) {
        return result; /* overflow guard */
    }

    long long* buf = (long long*)malloc(len * sizeof(long long));
    if (buf == NULL) {
        return result;
    }

    size_t count = 0;
    for (size_t idx = 0; idx < len; ++idx) {
        if (arr[idx].type == ITEM_INT && arr[idx].i >= 0) {
            buf[count++] = arr[idx].i;
        }
    }

    if (count == 0) {
        free(buf);
        return result;
    }

    long long* shrunk = (long long*)realloc(buf, count * sizeof(long long));
    result.data = (shrunk != NULL) ? shrunk : buf;
    result.len = count;
    return result;
}

void free_int_array(IntArray* a) {
    if (a != NULL && a->data != NULL) {
        free(a->data);
        a->data = NULL;
        a->len = 0;
    }
}

static void print_int_array(const IntArray* a) {
    printf("[");
    if (a != NULL && a->data != NULL) {
        for (size_t i = 0; i < a->len; ++i) {
            printf("%lld", a->data[i]);
            if (i + 1 < a->len) {
                printf(", ");
            }
        }
    }
    printf("]\n");
}

#define INT_ITEM(v) (Item){ITEM_INT, (long long)(v), NULL}
#define STR_ITEM(s) (Item){ITEM_STR, 0LL, (s)}

int main(void) {
    /* Test case 1 */
    Item t1[] = { INT_ITEM(1), INT_ITEM(2), STR_ITEM("a"), STR_ITEM("b") };
    IntArray r1 = filter_list(t1, sizeof(t1) / sizeof(t1[0]));
    print_int_array(&r1); /* expected: [1, 2] */
    free_int_array(&r1);

    /* Test case 2 */
    Item t2[] = { INT_ITEM(1), STR_ITEM("a"), STR_ITEM("b"), INT_ITEM(0), INT_ITEM(15) };
    IntArray r2 = filter_list(t2, sizeof(t2) / sizeof(t2[0]));
    print_int_array(&r2); /* expected: [1, 0, 15] */
    free_int_array(&r2);

    /* Test case 3 */
    Item t3[] = { INT_ITEM(1), INT_ITEM(2), STR_ITEM("a"), STR_ITEM("b"), STR_ITEM("aasf"), STR_ITEM("1"), STR_ITEM("123"), INT_ITEM(123) };
    IntArray r3 = filter_list(t3, sizeof(t3) / sizeof(t3[0]));
    print_int_array(&r3); /* expected: [1, 2, 123] */
    free_int_array(&r3);

    /* Test case 4 - only strings */
    Item t4[] = { STR_ITEM("x"), STR_ITEM("y"), STR_ITEM("z") };
    IntArray r4 = filter_list(t4, sizeof(t4) / sizeof(t4[0]));
    print_int_array(&r4); /* expected: [] */
    free_int_array(&r4);

    /* Test case 5 - includes a negative integer (validated out), and mixed entries */
    Item t5[] = { INT_ITEM(0), INT_ITEM(5), INT_ITEM(-3), STR_ITEM("x"), INT_ITEM(42), STR_ITEM("0") };
    IntArray r5 = filter_list(t5, sizeof(t5) / sizeof(t5[0]));
    print_int_array(&r5); /* expected: [0, 5, 42] */
    free_int_array(&r5);

    return 0;
}