
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int* data;
    int size;
} MemoryResource;

MemoryResource* createMemoryResource(int size) {
    MemoryResource* resource = (MemoryResource*)malloc(sizeof(MemoryResource));
    if (resource == NULL) {
        return NULL;
    }
    
    resource->size = size;
    resource->data = (int*)calloc(size, sizeof(int));
    if (resource->data == NULL) {
        free(resource);
        return NULL;
    }
    
    printf("Allocated memory for %d integers\\n", size);
    return resource;
}

void setData(MemoryResource* resource, int index, int value) {
    if (resource != NULL && index >= 0 && index < resource->size) {
        resource->data[index] = value;
    }
}

int getData(MemoryResource* resource, int index) {
    if (resource != NULL && index >= 0 && index < resource->size) {
        return resource->data[index];
    }
    return -1;
}

void deallocateMemoryResource(MemoryResource* resource) {
    if (resource != NULL) {
        if (resource->data != NULL) {
            printf("Deallocated memory for %d integers\\n", resource->size);
            free(resource->data);
            resource->data = NULL;
        }
        free(resource);
    }
}

MemoryResource* allocateMemory(int size) {
    return createMemoryResource(size);
}

void deallocateMemory(MemoryResource* resource) {
    deallocateMemoryResource(resource);
}

int main() {
    printf("Test Case 1: Allocate and deallocate 10 integers\\n");
    MemoryResource* res1 = allocateMemory(10);
    setData(res1, 0, 100);
    setData(res1, 5, 500);
    printf("Data at index 0: %d\\n", getData(res1, 0));
    printf("Data at index 5: %d\\n", getData(res1, 5));
    deallocateMemory(res1);
    printf("\\n");
    
    printf("Test Case 2: Allocate and deallocate 100 integers\\n");
    MemoryResource* res2 = allocateMemory(100);
    for (int i = 0; i < 10; i++) {
        setData(res2, i, i * 10);
    }
    printf("Data at index 7: %d\\n", getData(res2, 7));
    deallocateMemory(res2);
    printf("\\n");
    
    printf("Test Case 3: Allocate and deallocate 1 integer\\n");
    MemoryResource* res3 = allocateMemory(1);
    setData(res3, 0, 999);
    printf("Data at index 0: %d\\n", getData(res3, 0));
    deallocateMemory(res3);
    printf("\\n");
    
    printf("Test Case 4: Allocate and deallocate 1000 integers\\n");
    MemoryResource* res4 = allocateMemory(1000);
    setData(res4, 999, 12345);
    printf("Data at index 999: %d\\n", getData(res4, 999));
    deallocateMemory(res4);
    printf("\\n");
    
    printf("Test Case 5: Multiple allocations and deallocations\\n");
    MemoryResource* res5a = allocateMemory(50);
    MemoryResource* res5b = allocateMemory(75);
    setData(res5a, 25, 2500);
    setData(res5b, 50, 5000);
    printf("Resource A at index 25: %d\\n", getData(res5a, 25));
    printf("Resource B at index 50: %d\\n", getData(res5b, 50));
    deallocateMemory(res5a);
    deallocateMemory(res5b);
    
    return 0;
}
