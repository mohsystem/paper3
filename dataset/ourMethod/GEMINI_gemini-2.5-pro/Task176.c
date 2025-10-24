#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} PrefixProducts;

typedef struct {
    PrefixProducts prefixProducts;
} ProductOfNumbers;

void initPrefixProducts(PrefixProducts* p, size_t initial_capacity) {
    p->data = (int*)malloc(initial_capacity * sizeof(int));
    if (p->data == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    p->size = 0;
    p->capacity = initial_capacity;
}

void appendPrefixProduct(PrefixProducts* p, int value) {
    if (p->size == p->capacity) {
        size_t new_capacity = p->capacity == 0 ? 1 : p->capacity * 2;
        int* new_data = (int*)realloc(p->data, new_capacity * sizeof(int));
        if (new_data == NULL) {
            perror("Failed to reallocate memory");
            free(p->data);
            exit(EXIT_FAILURE);
        }
        p->data = new_data;
        p->capacity = new_capacity;
    }
    p->data[p->size++] = value;
}

void clearPrefixProducts(PrefixProducts* p) {
    p->size = 0;
}

void freePrefixProducts(PrefixProducts* p) {
    if (p->data) {
        free(p->data);
        p->data = NULL;
    }
    p->size = 0;
    p->capacity = 0;
}

ProductOfNumbers* productOfNumbersCreate() {
    ProductOfNumbers* obj = (ProductOfNumbers*)malloc(sizeof(ProductOfNumbers));
    if (obj == NULL) {
        perror("Failed to allocate memory for ProductOfNumbers");
        exit(EXIT_FAILURE);
    }
    initPrefixProducts(&obj->prefixProducts, 10);
    appendPrefixProduct(&obj->prefixProducts, 1);
    return obj;
}

void productOfNumbersAdd(ProductOfNumbers* obj, int num) {
    if (num == 0) {
        clearPrefixProducts(&obj->prefixProducts);
        appendPrefixProduct(&obj->prefixProducts, 1);
    } else {
        int lastProduct = obj->prefixProducts.data[obj->prefixProducts.size - 1];
        appendPrefixProduct(&obj->prefixProducts, lastProduct * num);
    }
}

int productOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    size_t n = obj->prefixProducts.size;
    if ((size_t)k >= n) {
        return 0;
    }
    int totalProduct = obj->prefixProducts.data[n - 1];
    int productBeforeK = obj->prefixProducts.data[n - 1 - k];
    return totalProduct / productBeforeK;
}

void productOfNumbersFree(ProductOfNumbers* obj) {
    if (obj) {
        freePrefixProducts(&obj->prefixProducts);
        free(obj);
    }
}

int main() {
    // Test Case 1 (from example)
    printf("--- Test Case 1 ---\n");
    ProductOfNumbers* pon1 = productOfNumbersCreate();
    productOfNumbersAdd(pon1, 3);
    productOfNumbersAdd(pon1, 0);
    productOfNumbersAdd(pon1, 2);
    productOfNumbersAdd(pon1, 5);
    productOfNumbersAdd(pon1, 4);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(pon1, 2)); // Expected: 20
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(pon1, 3)); // Expected: 40
    printf("getProduct(4): %d\n", productOfNumbersGetProduct(pon1, 4)); // Expected: 0
    productOfNumbersAdd(pon1, 8);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(pon1, 2)); // Expected: 32
    productOfNumbersFree(pon1);

    // Test Case 2
    printf("\n--- Test Case 2 ---\n");
    ProductOfNumbers* pon2 = productOfNumbersCreate();
    productOfNumbersAdd(pon2, 5);
    productOfNumbersAdd(pon2, 10);
    productOfNumbersAdd(pon2, 2);
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(pon2, 3)); // Expected: 100
    productOfNumbersFree(pon2);

    // Test Case 3
    printf("\n--- Test Case 3 ---\n");
    ProductOfNumbers* pon3 = productOfNumbersCreate();
    productOfNumbersAdd(pon3, 1);
    productOfNumbersAdd(pon3, 1);
    productOfNumbersAdd(pon3, 0);
    productOfNumbersAdd(pon3, 1);
    productOfNumbersAdd(pon3, 1);
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(pon3, 2)); // Expected: 1
    printf("getProduct(3): %d\n", productOfNumbersGetProduct(pon3, 3)); // Expected: 0
    productOfNumbersFree(pon3);

    // Test Case 4
    printf("\n--- Test Case 4 ---\n");
    ProductOfNumbers* pon4 = productOfNumbersCreate();
    productOfNumbersAdd(pon4, 7);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(pon4, 1)); // Expected: 7
    productOfNumbersFree(pon4);

    // Test Case 5
    printf("\n--- Test Case 5 ---\n");
    ProductOfNumbers* pon5 = productOfNumbersCreate();
    productOfNumbersAdd(pon5, 4);
    productOfNumbersAdd(pon5, 0);
    productOfNumbersAdd(pon5, 9);
    productOfNumbersAdd(pon5, 8);
    productOfNumbersAdd(pon5, 0);
    productOfNumbersAdd(pon5, 2);
    printf("getProduct(1): %d\n", productOfNumbersGetProduct(pon5, 1)); // Expected: 2
    printf("getProduct(2): %d\n", productOfNumbersGetProduct(pon5, 2)); // Expected: 0
    productOfNumbersFree(pon5);
    
    return 0;
}