
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

typedef struct {
    size_t size;
    unsigned char* data;
    bool allocated;
} MemoryResource;

MemoryResource* allocateMemory(size_t size) {
    if (size == 0 || size > SIZE_MAX / 2) {
        fprintf(stderr, "Invalid memory size\\n");
        return NULL;
    }
    
    MemoryResource* resource = (MemoryResource*)malloc(sizeof(MemoryResource));
    if (resource == NULL) {
        fprintf(stderr, "Failed to allocate MemoryResource structure\\n");
        return NULL;
    }
    
    resource->data = (unsigned char*)malloc(size);
    if (resource->data == NULL) {
        fprintf(stderr, "Failed to allocate memory\\n");
        free(resource);
        return NULL;
    }
    
    memset(resource->data, 0, size);
    resource->size = size;
    resource->allocated = true;
    
    return resource;
}

int deallocateMemory(MemoryResource* resource) {
    if (resource == NULL) {
        fprintf(stderr, "Resource cannot be NULL\\n");
        return -1;
    }
    
    if (!resource->allocated) {
        fprintf(stderr, "Resource already deallocated\\n");
        return -1;
    }
    
    if (resource->data != NULL) {
        // Clear sensitive data before deallocation
        memset(resource->data, 0, resource->size);
        free(resource->data);
        resource->data = NULL;
    }
    
    resource->allocated = false;
    free(resource);
    
    return 0;
}

int main() {
    printf("Test Case 1: Basic allocation and deallocation\\n");
    MemoryResource* res1 = allocateMemory(1024);
    if (res1 != NULL) {
        printf("Allocated: %zu bytes, Status: %d\\n", res1->size, res1->allocated);
        if (deallocateMemory(res1) == 0) {
            printf("Deallocated successfully\\n");
        }
    }
    
    printf("\\nTest Case 2: Multiple allocations\\n");
    MemoryResource* resources[3];
    for (int i = 0; i < 3; i++) {
        resources[i] = allocateMemory(512 * (i + 1));
        if (resources[i] != NULL) {
            printf("Allocated resource %d: %zu bytes\\n", i + 1, resources[i]->size);
        }
    }
    for (int i = 0; i < 3; i++) {
        if (resources[i] != NULL) {
            deallocateMemory(resources[i]);
        }
    }
    printf("All resources deallocated\\n");
    
    printf("\\nTest Case 3: Invalid size (zero)\\n");
    MemoryResource* res3 = allocateMemory(0);
    if (res3 == NULL) {
        printf("Caught expected error: allocation failed\\n");
    }
    
    printf("\\nTest Case 4: Double deallocation prevention\\n");
    MemoryResource* res4 = allocateMemory(2048);
    if (res4 != NULL) {
        res4->allocated = false;
        if (deallocateMemory(res4) != 0) {
            printf("Caught expected error: already deallocated\\n");
            free(res4);
        }
    }
    
    printf("\\nTest Case 5: NULL resource deallocation\\n");
    if (deallocateMemory(NULL) != 0) {
        printf("Caught expected error: NULL resource\\n");
    }
    
    return 0;
}
