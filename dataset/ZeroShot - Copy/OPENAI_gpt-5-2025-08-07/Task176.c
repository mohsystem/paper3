#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    long long* data;
    size_t size;
    size_t capacity;
} ProductOfNumbers;

static int ensure_capacity(ProductOfNumbers* obj, size_t min_cap) {
    if (obj->capacity >= min_cap) return 1;
    size_t new_cap = obj->capacity ? obj->capacity : 16;
    while (new_cap < min_cap) {
        if (new_cap > SIZE_MAX / 2) {
            new_cap = min_cap;
            break;
        }
        new_cap *= 2;
    }
    long long* nd = (long long*)realloc(obj->data, new_cap * sizeof(long long));
    if (!nd) return 0;
    obj->data = nd;
    obj->capacity = new_cap;
    return 1;
}

ProductOfNumbers* ProductOfNumbersCreate(void) {
    ProductOfNumbers* obj = (ProductOfNumbers*)calloc(1, sizeof(ProductOfNumbers));
    if (!obj) return NULL;
    obj->capacity = 0;
    obj->size = 0;
    obj->data = NULL;
    if (!ensure_capacity(obj, 1)) {
        free(obj);
        return NULL;
    }
    obj->data[0] = 1LL; // sentinel
    obj->size = 1;
    return obj;
}

void ProductOfNumbersAdd(ProductOfNumbers* obj, int num) {
    if (!obj) return;
    if (num == 0) {
        // reset to sentinel
        if (obj->size == 0) {
            if (!ensure_capacity(obj, 1)) return;
        }
        obj->data[0] = 1LL;
        obj->size = 1;
        return;
    }
    if (!ensure_capacity(obj, obj->size + 1)) return;
    long long last = obj->data[obj->size - 1];
    long long next = last * (long long)num;
    obj->data[obj->size++] = next;
}

int ProductOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    if (!obj) return 0;
    if (k < 0) return 0;
    size_t ks = (size_t)k;
    if (ks >= obj->size) {
        return 0;
    }
    size_t sz = obj->size;
    long long num = obj->data[sz - 1];
    long long den = obj->data[sz - 1 - ks];
    if (den == 0LL) return 0;
    long long res = num / den;
    return (int)res;
}

void ProductOfNumbersFree(ProductOfNumbers* obj) {
    if (!obj) return;
    free(obj->data);
    free(obj);
}

int main(void) {
    // Test case 1: Example from prompt
    ProductOfNumbers* p1 = ProductOfNumbersCreate();
    ProductOfNumbersAdd(p1, 3);
    ProductOfNumbersAdd(p1, 0);
    ProductOfNumbersAdd(p1, 2);
    ProductOfNumbersAdd(p1, 5);
    ProductOfNumbersAdd(p1, 4);
    printf("%d\n", ProductOfNumbersGetProduct(p1, 2)); // 20
    printf("%d\n", ProductOfNumbersGetProduct(p1, 3)); // 40
    printf("%d\n", ProductOfNumbersGetProduct(p1, 4)); // 0
    ProductOfNumbersAdd(p1, 8);
    printf("%d\n", ProductOfNumbersGetProduct(p1, 2)); // 32
    ProductOfNumbersFree(p1);

    // Test case 2: No zeros
    ProductOfNumbers* p2 = ProductOfNumbersCreate();
    ProductOfNumbersAdd(p2, 1);
    ProductOfNumbersAdd(p2, 2);
    ProductOfNumbersAdd(p2, 3);
    ProductOfNumbersAdd(p2, 4);
    printf("%d\n", ProductOfNumbersGetProduct(p2, 4)); // 24
    ProductOfNumbersFree(p2);

    // Test case 3: Multiple zeros with short queries
    ProductOfNumbers* p3 = ProductOfNumbersCreate();
    ProductOfNumbersAdd(p3, 0);
    ProductOfNumbersAdd(p3, 7);
    ProductOfNumbersAdd(p3, 0);
    ProductOfNumbersAdd(p3, 10);
    printf("%d\n", ProductOfNumbersGetProduct(p3, 1)); // 10
    printf("%d\n", ProductOfNumbersGetProduct(p3, 2)); // 0
    ProductOfNumbersFree(p3);

    // Test case 4: Larger k within segment
    ProductOfNumbers* p4 = ProductOfNumbersCreate();
    ProductOfNumbersAdd(p4, 2);
    ProductOfNumbersAdd(p4, 2);
    ProductOfNumbersAdd(p4, 2);
    ProductOfNumbersAdd(p4, 2);
    ProductOfNumbersAdd(p4, 2);
    printf("%d\n", ProductOfNumbersGetProduct(p4, 5)); // 32
    ProductOfNumbersFree(p4);

    // Test case 5: Zero boundary checks
    ProductOfNumbers* p5 = ProductOfNumbersCreate();
    ProductOfNumbersAdd(p5, 9);
    ProductOfNumbersAdd(p5, 9);
    ProductOfNumbersAdd(p5, 0);
    ProductOfNumbersAdd(p5, 9);
    ProductOfNumbersAdd(p5, 9);
    ProductOfNumbersAdd(p5, 9);
    printf("%d\n", ProductOfNumbersGetProduct(p5, 3)); // 729
    printf("%d\n", ProductOfNumbersGetProduct(p5, 4)); // 0
    ProductOfNumbersFree(p5);

    return 0;
}