#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { ITEM_INT = 0, ITEM_STR = 1 } ItemType;

typedef struct {
    ItemType type;
    int i;
    const char* s;
} Item;

typedef struct {
    int* data;
    size_t size;
} IntArray;

IntArray filter_list(const Item* items, size_t n) {
    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (items[i].type == ITEM_INT) {
            count++;
        }
    }
    IntArray result;
    result.size = count;
    result.data = (int*)malloc(sizeof(int) * count);
    if (!result.data && count > 0) {
        result.size = 0;
        return result;
    }
    size_t idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (items[i].type == ITEM_INT) {
            result.data[idx++] = items[i].i;
        }
    }
    return result;
}

void print_int_array(const IntArray* arr) {
    printf("[");
    for (size_t i = 0; i < arr->size; ++i) {
        printf("%d", arr->data[i]);
        if (i + 1 < arr->size) printf(", ");
    }
    printf("]\n");
}

Item make_int(int x) {
    Item it;
    it.type = ITEM_INT;
    it.i = x;
    it.s = NULL;
    return it;
}

Item make_str(const char* s) {
    Item it;
    it.type = ITEM_STR;
    it.i = 0;
    it.s = s;
    return it;
}

int main() {
    Item t1[] = { make_int(1), make_int(2), make_str("a"), make_str("b") };
    IntArray r1 = filter_list(t1, sizeof(t1)/sizeof(t1[0]));
    print_int_array(&r1);
    free(r1.data);

    Item t2[] = { make_int(1), make_str("a"), make_str("b"), make_int(0), make_int(15) };
    IntArray r2 = filter_list(t2, sizeof(t2)/sizeof(t2[0]));
    print_int_array(&r2);
    free(r2.data);

    Item t3[] = { make_int(1), make_int(2), make_str("a"), make_str("b"), make_str("aasf"), make_str("1"), make_str("123"), make_int(123) };
    IntArray r3 = filter_list(t3, sizeof(t3)/sizeof(t3[0]));
    print_int_array(&r3);
    free(r3.data);

    Item t4[] = { make_str("x"), make_str("y"), make_str("z") };
    IntArray r4 = filter_list(t4, sizeof(t4)/sizeof(t4[0]));
    print_int_array(&r4);
    free(r4.data);

    Item t5[] = { make_int(0), make_int(99), make_int(1000), make_str("1000"), make_str(""), make_int(5) };
    IntArray r5 = filter_list(t5, sizeof(t5)/sizeof(t5[0]));
    print_int_array(&r5);
    free(r5.data);

    return 0;
}