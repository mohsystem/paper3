
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    void* data;
    size_t size;
    bool allocated;
} MemoryResource;

MemoryResource* allocateMemory(size_t size) {
    if (size == 0) {
        fprintf(stderr, "Size must be positive\\n");
        return NULL;
    }
    
    MemoryResource* resource = (MemoryResource*)malloc(sizeof(MemoryResource));
    if (resource == NULL) {
        fprintf(stderr, "Failed to allocate MemoryResource structure\\n");
        return NULL;
    }
    
    resource->data = malloc(size);
    if (resource->data == NULL) {
        fprintf(stderr, "Failed to allocate memory\\n");
        free(resource);
        return NULL;
    }
    
    memset(resource->data, 0, size);
    resource->size = size;
    resource->allocated = true;
    printf("Memory allocated: %zu bytes\\n", size);
    
    return resource;
}

void deallocateMemory(MemoryResource* resource) {
    if (resource != NULL && resource->allocated) {
        if (resource->data != NULL) {
            free(resource->data);
            resource->data = NULL;
        }
        resource->allocated = false;
        printf("Memory deallocated\\n");
        free(resource);
    }
}

bool isAllocated(MemoryResource* resource) {
    return resource != NULL && resource->allocated;
}

size_t getSize(MemoryResource* resource) {
    return (resource != NULL && resource->allocated) ? resource->size : 0;
}

int main() {
    printf("Test Case 1: Allocate and deallocate 1024 bytes\\n");
    MemoryResource* res1 = allocateMemory(1024);
    printf("Is allocated: %d\\n", isAllocated(res1));
    deallocateMemory(res1);
    printf("\\n");
    
    printf("Test Case 2: Allocate and deallocate 2048 bytes\\n");
    MemoryResource* res2 = allocateMemory(2048);
    printf("Size: %zu\\n", getSize(res2));
    deallocateMemory(res2);
    printf("\\n");
    
    printf("Test Case 3: Multiple allocations\\n");
    MemoryResource* res3 = allocateMemory(512);
    MemoryResource* res4 = allocateMemory(256);
    deallocateMemory(res3);
    deallocateMemory(res4);
    printf("\\n");
    
    printf("Test Case 4: Deallocate null resource\\n");
    deallocateMemory(NULL);
    printf("No error on null deallocation\\n");
    printf("\\n");
    
    printf("Test Case 5: Large allocation\\n");
    MemoryResource* res5 = allocateMemory(1048576); // 1MB
    printf("Large allocation size: %zu\\n", getSize(res5));
    deallocateMemory(res5);
    
    return 0;
}
