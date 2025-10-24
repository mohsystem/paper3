#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    int *prefix;
    int len;
    int cap;
} ProductOfNumbers;

ProductOfNumbers* ProductOfNumbersCreate(void) {
    ProductOfNumbers* obj = (ProductOfNumbers*)malloc(sizeof(ProductOfNumbers));
    if (!obj) return NULL;
    obj->cap = 8;
    obj->len = 1;
    obj->prefix = (int*)malloc(sizeof(int) * obj->cap);
    if (!obj->prefix) {
        free(obj);
        return NULL;
    }
    obj->prefix[0] = 1;
    return obj;
}

void ProductOfNumbersFree(ProductOfNumbers* obj) {
    if (!obj) return;
    free(obj->prefix);
    free(obj);
}

static int ensure_capacity(ProductOfNumbers* obj, int need) {
    if (need <= obj->cap) return 1;
    int newCap = obj->cap;
    while (newCap < need) {
        if (newCap > INT_MAX / 2) {
            newCap = need; // fallback
            break;
        }
        newCap *= 2;
    }
    int* np = (int*)realloc(obj->prefix, sizeof(int) * newCap);
    if (!np) return 0;
    obj->prefix = np;
    obj->cap = newCap;
    return 1;
}

void ProductOfNumbersAdd(ProductOfNumbers* obj, int num) {
    if (!obj) return;
    if (num == 0) {
        obj->len = 1;
        obj->prefix[0] = 1;
        return;
    }
    if (!ensure_capacity(obj, obj->len + 1)) return;
    long long prod = (long long)obj->prefix[obj->len - 1] * (long long)num;
    obj->prefix[obj->len] = (int)prod; // safe per constraints
    obj->len += 1;
}

int ProductOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    if (!obj) return 0;
    if (k >= obj->len) return 0;
    int last = obj->prefix[obj->len - 1];
    int prev = obj->prefix[obj->len - 1 - k];
    return last / prev;
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
    // Test 1: Example provided
    {
        ProductOfNumbers* pon = ProductOfNumbersCreate();
        ProductOfNumbersAdd(pon, 3);
        ProductOfNumbersAdd(pon, 0);
        ProductOfNumbersAdd(pon, 2);
        ProductOfNumbersAdd(pon, 5);
        ProductOfNumbersAdd(pon, 4);
        int out[4];
        out[0] = ProductOfNumbersGetProduct(pon, 2); // 20
        out[1] = ProductOfNumbersGetProduct(pon, 3); // 40
        out[2] = ProductOfNumbersGetProduct(pon, 4); // 0
        ProductOfNumbersAdd(pon, 8);
        out[3] = ProductOfNumbersGetProduct(pon, 2); // 32
        print_array(out, 4);
        ProductOfNumbersFree(pon);
    }

    // Test 2: No zeros
    {
        ProductOfNumbers* pon = ProductOfNumbersCreate();
        ProductOfNumbersAdd(pon, 1);
        ProductOfNumbersAdd(pon, 2);
        ProductOfNumbersAdd(pon, 3);
        ProductOfNumbersAdd(pon, 4);
        int out[3];
        out[0] = ProductOfNumbersGetProduct(pon, 1); // 4
        out[1] = ProductOfNumbersGetProduct(pon, 3); // 24
        out[2] = ProductOfNumbersGetProduct(pon, 4); // 24
        print_array(out, 3);
        ProductOfNumbersFree(pon);
    }

    // Test 3: Zeros resetting
    {
        ProductOfNumbers* pon = ProductOfNumbersCreate();
        ProductOfNumbersAdd(pon, 0);
        ProductOfNumbersAdd(pon, 0);
        ProductOfNumbersAdd(pon, 7);
        ProductOfNumbersAdd(pon, 3);
        int out[2];
        out[0] = ProductOfNumbersGetProduct(pon, 1); // 3
        out[1] = ProductOfNumbersGetProduct(pon, 2); // 21
        print_array(out, 2);
        ProductOfNumbersFree(pon);
    }

    // Test 4: Long k equals length
    {
        ProductOfNumbers* pon = ProductOfNumbersCreate();
        ProductOfNumbersAdd(pon, 2);
        ProductOfNumbersAdd(pon, 2);
        ProductOfNumbersAdd(pon, 2);
        ProductOfNumbersAdd(pon, 2);
        ProductOfNumbersAdd(pon, 2);
        int out[1];
        out[0] = ProductOfNumbersGetProduct(pon, 5); // 32
        print_array(out, 1);
        ProductOfNumbersFree(pon);
    }

    // Test 5: Mix with ones and zero
    {
        ProductOfNumbers* pon = ProductOfNumbersCreate();
        ProductOfNumbersAdd(pon, 1);
        ProductOfNumbersAdd(pon, 1);
        ProductOfNumbersAdd(pon, 1);
        ProductOfNumbersAdd(pon, 1);
        ProductOfNumbersAdd(pon, 0);
        ProductOfNumbersAdd(pon, 10);
        int out[3];
        out[0] = ProductOfNumbersGetProduct(pon, 1); // 10
        out[1] = ProductOfNumbersGetProduct(pon, 2); // 0
        ProductOfNumbersAdd(pon, 2);
        out[2] = ProductOfNumbersGetProduct(pon, 2); // 20
        print_array(out, 3);
        ProductOfNumbersFree(pon);
    }

    return 0;
}