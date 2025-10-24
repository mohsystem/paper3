
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* prefixProducts;
    int size;
    int capacity;
} Task176;

Task176* createTask176() {
    Task176* obj = (Task176*)malloc(sizeof(Task176));
    obj->capacity = 10;
    obj->size = 1;
    obj->prefixProducts = (int*)malloc(obj->capacity * sizeof(int));
    obj->prefixProducts[0] = 1;
    return obj;
}

void add(Task176* obj, int num) {
    if (num == 0) {
        // Reset the prefix products when 0 is encountered
        obj->size = 1;
        obj->prefixProducts[0] = 1;
    } else {
        // Expand capacity if needed
        if (obj->size >= obj->capacity) {
            obj->capacity *= 2;
            obj->prefixProducts = (int*)realloc(obj->prefixProducts, obj->capacity * sizeof(int));
        }
        // Add the product of last prefix and current number
        int lastProduct = obj->prefixProducts[obj->size - 1];
        obj->prefixProducts[obj->size] = lastProduct * num;
        obj->size++;
    }
}

int getProduct(Task176* obj, int k) {
    int n = obj->size;
    // If k is greater than or equal to the size, it means we crossed a zero
    if (k >= n) {
        return 0;
    }
    // Product of last k numbers = prefixProducts[n-1] / prefixProducts[n-1-k]
    return obj->prefixProducts[n - 1] / obj->prefixProducts[n - 1 - k];
}

void freeTask176(Task176* obj) {
    free(obj->prefixProducts);
    free(obj);
}

int main() {
    // Test case 1: Example from prompt
    Task176* test1 = createTask176();
    add(test1, 3);
    add(test1, 0);
    add(test1, 2);
    add(test1, 5);
    add(test1, 4);
    printf("Test 1 - getProduct(2): %d\\n", getProduct(test1, 2)); // Expected: 20
    printf("Test 1 - getProduct(3): %d\\n", getProduct(test1, 3)); // Expected: 40
    printf("Test 1 - getProduct(4): %d\\n", getProduct(test1, 4)); // Expected: 0
    add(test1, 8);
    printf("Test 1 - getProduct(2): %d\\n", getProduct(test1, 2)); // Expected: 32
    freeTask176(test1);
    
    // Test case 2: No zeros
    Task176* test2 = createTask176();
    add(test2, 2);
    add(test2, 3);
    add(test2, 4);
    printf("\\nTest 2 - getProduct(2): %d\\n", getProduct(test2, 2)); // Expected: 12
    printf("Test 2 - getProduct(3): %d\\n", getProduct(test2, 3)); // Expected: 24
    freeTask176(test2);
    
    // Test case 3: Multiple zeros
    Task176* test3 = createTask176();
    add(test3, 1);
    add(test3, 0);
    add(test3, 5);
    add(test3, 0);
    add(test3, 3);
    printf("\\nTest 3 - getProduct(1): %d\\n", getProduct(test3, 1)); // Expected: 3
    freeTask176(test3);
    
    // Test case 4: All ones
    Task176* test4 = createTask176();
    add(test4, 1);
    add(test4, 1);
    add(test4, 1);
    printf("\\nTest 4 - getProduct(3): %d\\n", getProduct(test4, 3)); // Expected: 1
    freeTask176(test4);
    
    // Test case 5: Large numbers
    Task176* test5 = createTask176();
    add(test5, 10);
    add(test5, 10);
    add(test5, 10);
    printf("\\nTest 5 - getProduct(2): %d\\n", getProduct(test5, 2)); // Expected: 100
    printf("Test 5 - getProduct(3): %d\\n", getProduct(test5, 3)); // Expected: 1000
    freeTask176(test5);
    
    return 0;
}
