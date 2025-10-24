#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 40001

typedef struct {
    int prefix_products[MAX_SIZE];
    int size;
} ProductOfNumbers;

void productOfNumbersInit(ProductOfNumbers* obj) {
    obj->size = 1;
    obj->prefix_products[0] = 1;
}

void productOfNumbersAdd(ProductOfNumbers* obj, int num) {
    if (num == 0) {
        obj->size = 1;
        obj->prefix_products[0] = 1;
    } else {
        if (obj->size < MAX_SIZE) {
            obj->prefix_products[obj->size] = obj->prefix_products[obj->size - 1] * num;
            obj->size++;
        }
    }
}

int productOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    int n = obj->size;
    if (k >= n) {
        return 0;
    }
    return obj->prefix_products[n - 1] / obj->prefix_products[n - 1 - k];
}


int main() {
    // Test Case 1
    ProductOfNumbers pon1;
    productOfNumbersInit(&pon1);
    productOfNumbersAdd(&pon1, 3);
    productOfNumbersAdd(&pon1, 0);
    productOfNumbersAdd(&pon1, 2);
    productOfNumbersAdd(&pon1, 5);
    productOfNumbersAdd(&pon1, 4);
    printf("%d\n", productOfNumbersGetProduct(&pon1, 2)); // 20
    printf("%d\n", productOfNumbersGetProduct(&pon1, 3)); // 40
    printf("%d\n", productOfNumbersGetProduct(&pon1, 4)); // 0
    productOfNumbersAdd(&pon1, 8);
    printf("%d\n", productOfNumbersGetProduct(&pon1, 2)); // 32
    printf("\n");

    // Test Case 2
    ProductOfNumbers pon2;
    productOfNumbersInit(&pon2);
    productOfNumbersAdd(&pon2, 1);
    productOfNumbersAdd(&pon2, 2);
    productOfNumbersAdd(&pon2, 3);
    productOfNumbersAdd(&pon2, 4);
    productOfNumbersAdd(&pon2, 5);
    printf("%d\n", productOfNumbersGetProduct(&pon2, 1)); // 5
    printf("%d\n", productOfNumbersGetProduct(&pon2, 2)); // 20
    printf("%d\n", productOfNumbersGetProduct(&pon2, 5)); // 120
    printf("\n");
    
    // Test Case 3
    ProductOfNumbers pon3;
    productOfNumbersInit(&pon3);
    productOfNumbersAdd(&pon3, 0);
    productOfNumbersAdd(&pon3, 10);
    productOfNumbersAdd(&pon3, 20);
    printf("%d\n", productOfNumbersGetProduct(&pon3, 1)); // 20
    printf("%d\n", productOfNumbersGetProduct(&pon3, 2)); // 200
    printf("%d\n", productOfNumbersGetProduct(&pon3, 3)); // 0
    printf("\n");

    // Test Case 4
    ProductOfNumbers pon4;
    productOfNumbersInit(&pon4);
    productOfNumbersAdd(&pon4, 5);
    productOfNumbersAdd(&pon4, 6);
    productOfNumbersAdd(&pon4, 0);
    productOfNumbersAdd(&pon4, 7);
    productOfNumbersAdd(&pon4, 0);
    productOfNumbersAdd(&pon4, 8);
    productOfNumbersAdd(&pon4, 9);
    printf("%d\n", productOfNumbersGetProduct(&pon4, 1)); // 9
    printf("%d\n", productOfNumbersGetProduct(&pon4, 2)); // 72
    printf("%d\n", productOfNumbersGetProduct(&pon4, 3)); // 0
    printf("\n");
    
    // Test Case 5
    ProductOfNumbers pon5;
    productOfNumbersInit(&pon5);
    productOfNumbersAdd(&pon5, 42);
    printf("%d\n", productOfNumbersGetProduct(&pon5, 1)); // 42
    productOfNumbersAdd(&pon5, 0);
    printf("%d\n", productOfNumbersGetProduct(&pon5, 1)); // 0
    productOfNumbersAdd(&pon5, 1);
    printf("%d\n", productOfNumbersGetProduct(&pon5, 1)); // 1
    printf("%d\n", productOfNumbersGetProduct(&pon5, 2)); // 0
    printf("\n");
    
    return 0;
}