
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE 100
#define MAX_BUFFER_SIZE 1000000

typedef struct {
    int* data;
    int size;
} Buffer;

Buffer* allocateBuffer(int size) {
    if (size <= 0 || size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Invalid buffer size\\n");
        return NULL;
    }
    
    Buffer* buffer = (Buffer*)malloc(sizeof(Buffer));
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate buffer structure\\n");
        return NULL;
    }
    
    buffer->data = (int*)calloc(size, sizeof(int));
    if (buffer->data == NULL) {
        fprintf(stderr, "Failed to allocate buffer data\\n");
        free(buffer);
        return NULL;
    }
    
    buffer->size = size;
    return buffer;
}

void initializeBuffer(Buffer* buffer) {
    if (buffer == NULL || buffer->data == NULL) {
        fprintf(stderr, "Invalid buffer\\n");
        return;
    }
    
    for (int i = 0; i < buffer->size; i++) {
        buffer->data[i] = i * 10;
    }
}

bool readFromBuffer(Buffer* buffer, int index, int* result) {
    if (buffer == NULL || buffer->data == NULL) {
        fprintf(stderr, "Buffer cannot be NULL\\n");
        return false;
    }
    
    if (index < 0 || index >= buffer->size) {
        fprintf(stderr, "Index out of bounds: %d\\n", index);
        return false;
    }
    
    *result = buffer->data[index];
    return true;
}

void freeBuffer(Buffer* buffer) {
    if (buffer != NULL) {
        if (buffer->data != NULL) {
            free(buffer->data);
        }
        free(buffer);
    }
}

int main() {
    printf("=== Secure Memory Buffer Access Test Cases ===\\n\\n");
    
    int result;
    
    // Test Case 1: Normal valid access
    printf("Test 1 - Valid access at index 50: ");
    Buffer* buffer1 = allocateBuffer(BUFFER_SIZE);
    if (buffer1 != NULL) {
        initializeBuffer(buffer1);
        if (readFromBuffer(buffer1, 50, &result)) {
            printf("%d\\n", result);
        }
        freeBuffer(buffer1);
    }
    
    // Test Case 2: Access at boundary (first element)
    printf("Test 2 - Valid access at index 0: ");
    Buffer* buffer2 = allocateBuffer(BUFFER_SIZE);
    if (buffer2 != NULL) {
        initializeBuffer(buffer2);
        if (readFromBuffer(buffer2, 0, &result)) {
            printf("%d\\n", result);
        }
        freeBuffer(buffer2);
    }
    
    // Test Case 3: Access at boundary (last element)
    printf("Test 3 - Valid access at index %d: ", BUFFER_SIZE - 1);
    Buffer* buffer3 = allocateBuffer(BUFFER_SIZE);
    if (buffer3 != NULL) {
        initializeBuffer(buffer3);
        if (readFromBuffer(buffer3, BUFFER_SIZE - 1, &result)) {
            printf("%d\\n", result);
        }
        freeBuffer(buffer3);
    }
    
    // Test Case 4: Invalid access (negative index)
    printf("Test 4 - Access at index -5 (Expected Error): ");
    Buffer* buffer4 = allocateBuffer(BUFFER_SIZE);
    if (buffer4 != NULL) {
        initializeBuffer(buffer4);
        if (!readFromBuffer(buffer4, -5, &result)) {
            printf("Correctly rejected\\n");
        }
        freeBuffer(buffer4);
    }
    
    // Test Case 5: Invalid access (out of bounds)
    printf("Test 5 - Access at index 150 (Expected Error): ");
    Buffer* buffer5 = allocateBuffer(BUFFER_SIZE);
    if (buffer5 != NULL) {
        initializeBuffer(buffer5);
        if (!readFromBuffer(buffer5, 150, &result)) {
            printf("Correctly rejected\\n");
        }
        freeBuffer(buffer5);
    }
    
    return 0;
}
