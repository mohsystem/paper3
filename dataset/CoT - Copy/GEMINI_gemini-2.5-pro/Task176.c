#include <stdio.h>

#define MAX_SIZE 40002

typedef struct {
    int prefixProducts[MAX_SIZE];
    int size;
} ProductOfNumbers;

void productOfNumbersCreate(ProductOfNumbers* obj) {
    obj->prefixProducts[0] = 1;
    obj->size = 1;
}

void productOfNumbersAdd(ProductOfNumbers* obj, int num) {
    if (num == 0) {
        obj->prefixProducts[0] = 1;
        obj->size = 1;
    } else {
        if (obj->size < MAX_SIZE) {
            obj->prefixProducts[obj->size] = obj->prefixProducts[obj->size - 1] * num;
            obj->size++;
        }
    }
}

int productOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    int n = obj->size;
    if (k >= n) {
        return 0;
    }
    return obj->prefixProducts[n - 1] / obj->prefixProducts[n - 1 - k];
}


int main() {
    // Test Case 1: Example from prompt
    printf("Test Case 1:\n");
    ProductOfNumbers p1;
    productOfNumbersCreate(&p1);
    productOfNumbersAdd(&p1, 3);
    productOfNumbersAdd(&p1, 0);
    productOfNumbersAdd(&p1, 2);
    productOfNumbersAdd(&p1, 5);
    productOfNumbersAdd(&p1, 4);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(&p1, 2));
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(&p1, 3));
    printf("getProduct(4): %d\n", productOfNumbersGetProduct(&p1, 4));
    productOfNumbersAdd(&p1, 8);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(&p1, 2));
    printf("\n");

    // Test Case 2: Multiple zeros
    printf("Test Case 2:\n");
    ProductOfNumbers p2;
    productOfNumbersCreate(&p2);
    productOfNumbersAdd(&p2, 1);
    productOfNumbersAdd(&p2, 2);
    productOfNumbersAdd(&p2, 0);
    productOfNumbersAdd(&p2, 4);
    productOfNumbersAdd(&p2, 5);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(&p2, 2));
    productOfNumbersAdd(&p2, 0);
    productOfNumbersAdd(&p2, 10);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(&p2, 1));
    printf("\n");
    
    // Test Case 3: k equals the number of non-zero elements
    printf("Test Case 3:\n");
    ProductOfNumbers p3;
    productOfNumbersCreate(&p3);
    productOfNumbersAdd(&p3, 5);
    productOfNumbersAdd(&p3, 6);
    productOfNumbersAdd(&p3, 7);
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(&p3, 3));
    printf("\n");
    
    // Test Case 4: getProduct(1)
    printf("Test Case 4:\n");
    ProductOfNumbers p4;
    productOfNumbersCreate(&p4);
    productOfNumbersAdd(&p4, 100);
    productOfNumbersAdd(&p4, 50);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(&p4, 1));
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(&p4, 2));
    printf("\n");
    
    // Test Case 5: k spans across a zero
    printf("Test Case 5:\n");
    ProductOfNumbers p5;
    productOfNumbersCreate(&p5);
    productOfNumbersAdd(&p5, 1);
    productOfNumbersAdd(&p5, 2);
    productOfNumbersAdd(&p5, 3);
    productOfNumbersAdd(&p5, 0);
    productOfNumbersAdd(&p5, 4);
    productOfNumbersAdd(&p5, 5);
    productOfNumbersAdd(&p5, 6);
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(&p5, 3));
    printf("getProduct(4): %d\n", productOfNumbersGetProduct(&p5, 4));
    printf("\n");

    return 0;
}