#include <stdio.h>
#include <stdlib.h>

// Since C is not object-oriented, we use a struct to hold the data
// and functions that operate on that struct.
typedef struct {
    int* prefixProducts;
    int size;
    int capacity;
} ProductOfNumbers;

// This function acts as a constructor.
ProductOfNumbers* productOfNumbersCreate() {
    ProductOfNumbers* obj = (ProductOfNumbers*)malloc(sizeof(ProductOfNumbers));
    obj->capacity = 10;
    obj->prefixProducts = (int*)malloc(obj->capacity * sizeof(int));
    obj->size = 1;
    obj->prefixProducts[0] = 1;
    return obj;
}

// This function adds a number to the stream.
void productOfNumbersAdd(ProductOfNumbers* obj, int num) {
    if (num == 0) {
        obj->size = 1;
        obj->prefixProducts[0] = 1;
    } else {
        // Resize the array if necessary.
        if (obj->size == obj->capacity) {
            obj->capacity *= 2;
            obj->prefixProducts = (int*)realloc(obj->prefixProducts, obj->capacity * sizeof(int));
        }
        obj->prefixProducts[obj->size] = obj->prefixProducts[obj->size - 1] * num;
        obj->size++;
    }
}

// This function gets the product of the last k numbers.
int productOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    int n = obj->size;
    if (k >= n) {
        return 0;
    }
    return obj->prefixProducts[n - 1] / obj->prefixProducts[n - 1 - k];
}

// This function acts as a destructor to free memory.
void productOfNumbersFree(ProductOfNumbers* obj) {
    if (obj) {
        free(obj->prefixProducts);
        free(obj);
    }
}

// C doesn't have a 'main class' concept like Java.
// So we create a function to run tests and call it from main.
void run_tests() {
    // Test Case 1 (from example)
    printf("Test Case 1:\n");
    ProductOfNumbers* p1 = productOfNumbersCreate();
    productOfNumbersAdd(p1, 3);
    productOfNumbersAdd(p1, 0);
    productOfNumbersAdd(p1, 2);
    productOfNumbersAdd(p1, 5);
    productOfNumbersAdd(p1, 4);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(p1, 2));
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(p1, 3));
    printf("getProduct(4): %d\n", productOfNumbersGetProduct(p1, 4));
    productOfNumbersAdd(p1, 8);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(p1, 2));
    productOfNumbersFree(p1);
    printf("\n");

    // Test Case 2: No zeros
    printf("Test Case 2:\n");
    ProductOfNumbers* p2 = productOfNumbersCreate();
    productOfNumbersAdd(p2, 1);
    productOfNumbersAdd(p2, 2);
    productOfNumbersAdd(p2, 3);
    productOfNumbersAdd(p2, 4);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(p2, 1));
    printf("getProduct(4): %d\n", productOfNumbersGetProduct(p2, 4));
    productOfNumbersFree(p2);
    printf("\n");

    // Test Case 3: Starts with zero
    printf("Test Case 3:\n");
    ProductOfNumbers* p3 = productOfNumbersCreate();
    productOfNumbersAdd(p3, 0);
    productOfNumbersAdd(p3, 5);
    productOfNumbersAdd(p3, 10);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(p3, 1));
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(p3, 2));
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(p3, 3));
    productOfNumbersFree(p3);
    printf("\n");

    // Test Case 4: Multiple zeros
    printf("Test Case 4:\n");
    ProductOfNumbers* p4 = productOfNumbersCreate();
    productOfNumbersAdd(p4, 5);
    productOfNumbersAdd(p4, 6);
    productOfNumbersAdd(p4, 0);
    productOfNumbersAdd(p4, 7);
    productOfNumbersAdd(p4, 0);
    productOfNumbersAdd(p4, 8);
    productOfNumbersAdd(p4, 9);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(p4, 1));
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(p4, 2));
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(p4, 3));
    productOfNumbersFree(p4);
    printf("\n");

    // Test Case 5: k equals the number of elements
    printf("Test Case 5:\n");
    ProductOfNumbers* p5 = productOfNumbersCreate();
    productOfNumbersAdd(p5, 10);
    productOfNumbersAdd(p5, 20);
    productOfNumbersAdd(p5, 30);
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(p5, 3));
    productOfNumbersFree(p5);
    printf("\n");
}

int main() {
    run_tests();
    return 0;
}