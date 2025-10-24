
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* prefixProducts;
    int size;
    int capacity;
} Task176;

Task176* Task176Create() {
    Task176* obj = (Task176*)malloc(sizeof(Task176));
    obj->capacity = 10;
    obj->size = 1;
    obj->prefixProducts = (int*)malloc(obj->capacity * sizeof(int));
    obj->prefixProducts[0] = 1;
    return obj;
}

void Task176Add(Task176* obj, int num) {
    if (num == 0) {
        obj->size = 1;
        obj->prefixProducts[0] = 1;
    } else {
        if (obj->size >= obj->capacity) {
            obj->capacity *= 2;
            obj->prefixProducts = (int*)realloc(obj->prefixProducts, obj->capacity * sizeof(int));
        }
        int lastProduct = obj->prefixProducts[obj->size - 1];
        obj->prefixProducts[obj->size] = lastProduct * num;
        obj->size++;
    }
}

int Task176GetProduct(Task176* obj, int k) {
    if (k >= obj->size) {
        return 0;
    }
    return obj->prefixProducts[obj->size - 1] / obj->prefixProducts[obj->size - 1 - k];
}

void Task176Free(Task176* obj) {
    free(obj->prefixProducts);
    free(obj);
}

int main() {
    // Test case 1: Example from problem
    Task176* test1 = Task176Create();
    Task176Add(test1, 3);
    Task176Add(test1, 0);
    Task176Add(test1, 2);
    Task176Add(test1, 5);
    Task176Add(test1, 4);
    printf("%d\\n", Task176GetProduct(test1, 2)); // Expected: 20
    printf("%d\\n", Task176GetProduct(test1, 3)); // Expected: 40
    printf("%d\\n", Task176GetProduct(test1, 4)); // Expected: 0
    Task176Add(test1, 8);
    printf("%d\\n", Task176GetProduct(test1, 2)); // Expected: 32
    Task176Free(test1);
    
    // Test case 2: All non-zero
    Task176* test2 = Task176Create();
    Task176Add(test2, 2);
    Task176Add(test2, 3);
    Task176Add(test2, 4);
    printf("%d\\n", Task176GetProduct(test2, 2)); // Expected: 12
    printf("%d\\n", Task176GetProduct(test2, 3)); // Expected: 24
    Task176Free(test2);
    
    // Test case 3: Multiple zeros
    Task176* test3 = Task176Create();
    Task176Add(test3, 5);
    Task176Add(test3, 0);
    Task176Add(test3, 0);
    Task176Add(test3, 6);
    printf("%d\\n", Task176GetProduct(test3, 1)); // Expected: 6
    Task176Free(test3);
    
    // Test case 4: Single element
    Task176* test4 = Task176Create();
    Task176Add(test4, 7);
    printf("%d\\n", Task176GetProduct(test4, 1)); // Expected: 7
    Task176Free(test4);
    
    // Test case 5: Zero then non-zero
    Task176* test5 = Task176Create();
    Task176Add(test5, 1);
    Task176Add(test5, 2);
    Task176Add(test5, 0);
    Task176Add(test5, 3);
    Task176Add(test5, 4);
    printf("%d\\n", Task176GetProduct(test5, 2)); // Expected: 12
    printf("%d\\n", Task176GetProduct(test5, 3)); // Expected: 0
    Task176Free(test5);
    
    return 0;
}
