
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* prefixProducts;
    int size;
    int capacity;
} Task176;

Task176* Task176Create() {
    Task176* obj = (Task176*)malloc(sizeof(Task176));
    if (obj == NULL) return NULL;
    
    obj->capacity = 10;
    obj->size = 1;
    obj->prefixProducts = (int*)malloc(obj->capacity * sizeof(int));
    if (obj->prefixProducts == NULL) {
        free(obj);
        return NULL;
    }
    obj->prefixProducts[0] = 1;
    return obj;
}

void Task176Add(Task176* obj, int num) {
    if (obj == NULL) return;
    
    if (num == 0) {
        obj->size = 1;
        obj->prefixProducts[0] = 1;
    } else {
        if (obj->size >= obj->capacity) {
            obj->capacity *= 2;
            int* newArray = (int*)realloc(obj->prefixProducts, obj->capacity * sizeof(int));
            if (newArray == NULL) return;
            obj->prefixProducts = newArray;
        }
        int lastProduct = obj->prefixProducts[obj->size - 1];
        obj->prefixProducts[obj->size] = lastProduct * num;
        obj->size++;
    }
}

int Task176GetProduct(Task176* obj, int k) {
    if (obj == NULL) return 0;
    
    int n = obj->size;
    if (k >= n) {
        return 0;
    }
    return obj->prefixProducts[n - 1] / obj->prefixProducts[n - 1 - k];
}

void Task176Free(Task176* obj) {
    if (obj != NULL) {
        if (obj->prefixProducts != NULL) {
            free(obj->prefixProducts);
        }
        free(obj);
    }
}

int main() {
    // Test case 1: Basic example from problem
    Task176* test1 = Task176Create();
    Task176Add(test1, 3);
    Task176Add(test1, 0);
    Task176Add(test1, 2);
    Task176Add(test1, 5);
    Task176Add(test1, 4);
    printf("Test 1 - getProduct(2): %d\\n", Task176GetProduct(test1, 2)); // Expected: 20
    printf("Test 1 - getProduct(3): %d\\n", Task176GetProduct(test1, 3)); // Expected: 40
    printf("Test 1 - getProduct(4): %d\\n", Task176GetProduct(test1, 4)); // Expected: 0
    Task176Add(test1, 8);
    printf("Test 1 - getProduct(2): %d\\n", Task176GetProduct(test1, 2)); // Expected: 32
    Task176Free(test1);
    
    // Test case 2: All non-zero numbers
    Task176* test2 = Task176Create();
    Task176Add(test2, 2);
    Task176Add(test2, 3);
    Task176Add(test2, 4);
    printf("\\nTest 2 - getProduct(2): %d\\n", Task176GetProduct(test2, 2)); // Expected: 12
    printf("Test 2 - getProduct(3): %d\\n", Task176GetProduct(test2, 3)); // Expected: 24
    Task176Free(test2);
    
    // Test case 3: Single element
    Task176* test3 = Task176Create();
    Task176Add(test3, 5);
    printf("\\nTest 3 - getProduct(1): %d\\n", Task176GetProduct(test3, 1)); // Expected: 5
    Task176Free(test3);
    
    // Test case 4: Multiple zeros
    Task176* test4 = Task176Create();
    Task176Add(test4, 1);
    Task176Add(test4, 0);
    Task176Add(test4, 2);
    Task176Add(test4, 0);
    Task176Add(test4, 3);
    printf("\\nTest 4 - getProduct(1): %d\\n", Task176GetProduct(test4, 1)); // Expected: 3
    printf("Test 4 - getProduct(2): %d\\n", Task176GetProduct(test4, 2)); // Expected: 0
    Task176Free(test4);
    
    // Test case 5: Large numbers
    Task176* test5 = Task176Create();
    Task176Add(test5, 10);
    Task176Add(test5, 20);
    Task176Add(test5, 30);
    printf("\\nTest 5 - getProduct(2): %d\\n", Task176GetProduct(test5, 2)); // Expected: 600
    printf("Test 5 - getProduct(3): %d\\n", Task176GetProduct(test5, 3)); // Expected: 6000
    Task176Free(test5);
    
    return 0;
}
