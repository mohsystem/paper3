#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *arr;
    int size;
    int capacity;
} ProductOfNumbers;

void initProduct(ProductOfNumbers *p) {
    p->capacity = 16;
    p->arr = (int*)malloc(sizeof(int) * p->capacity);
    p->size = 1;
    p->arr[0] = 1;
}

void freeProduct(ProductOfNumbers *p) {
    if (p->arr) free(p->arr);
    p->arr = NULL;
    p->size = 0;
    p->capacity = 0;
}

void addProduct(ProductOfNumbers *p, int num) {
    if (num == 0) {
        p->size = 1;
        p->arr[0] = 1;
        return;
    }
    if (p->size >= p->capacity) {
        p->capacity *= 2;
        p->arr = (int*)realloc(p->arr, sizeof(int) * p->capacity);
    }
    int last = p->arr[p->size - 1];
    p->arr[p->size] = last * num;
    p->size += 1;
}

int getProductK(ProductOfNumbers *p, int k) {
    if (k >= p->size) return 0;
    int n = p->size;
    return p->arr[n - 1] / p->arr[n - 1 - k];
}

int main() {
    // Test case 1 (sample)
    ProductOfNumbers p1; initProduct(&p1);
    addProduct(&p1, 3); addProduct(&p1, 0); addProduct(&p1, 2); addProduct(&p1, 5); addProduct(&p1, 4);
    printf("%d\n", getProductK(&p1, 2));
    printf("%d\n", getProductK(&p1, 3));
    printf("%d\n", getProductK(&p1, 4));
    addProduct(&p1, 8);
    printf("%d\n", getProductK(&p1, 2));
    freeProduct(&p1);

    // Test case 2
    ProductOfNumbers p2; initProduct(&p2);
    addProduct(&p2, 1); addProduct(&p2, 2); addProduct(&p2, 3); addProduct(&p2, 4);
    printf("%d\n", getProductK(&p2, 1));
    printf("%d\n", getProductK(&p2, 4));
    freeProduct(&p2);

    // Test case 3
    ProductOfNumbers p3; initProduct(&p3);
    addProduct(&p3, 0); addProduct(&p3, 0); addProduct(&p3, 2); addProduct(&p3, 3);
    printf("%d\n", getProductK(&p3, 1));
    printf("%d\n", getProductK(&p3, 3));
    freeProduct(&p3);

    // Test case 4
    ProductOfNumbers p4; initProduct(&p4);
    addProduct(&p4, 10); addProduct(&p4, 10); addProduct(&p4, 10);
    printf("%d\n", getProductK(&p4, 2));
    printf("%d\n", getProductK(&p4, 3));
    freeProduct(&p4);

    // Test case 5
    ProductOfNumbers p5; initProduct(&p5);
    addProduct(&p5, 100); addProduct(&p5, 0); addProduct(&p5, 100); addProduct(&p5, 100);
    printf("%d\n", getProductK(&p5, 1));
    printf("%d\n", getProductK(&p5, 2));
    printf("%d\n", getProductK(&p5, 3));
    freeProduct(&p5);

    return 0;
}